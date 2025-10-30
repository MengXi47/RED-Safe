"""Stream worker logic for fall detection."""

from __future__ import annotations

import contextlib
import logging
import math
import os
import threading
import time
from collections import defaultdict, deque
from typing import Deque, Dict, List

import cv2
import numpy as np
from ultralytics import YOLO

from .api import send_windows_to_api
from .config import AppConfig
from .db import IPCStreamConfig

LOGGER = logging.getLogger(__name__)


def _show_window(window_name: str, img: np.ndarray, last_time: float) -> tuple[float, bool]:
    now = time.time()
    fps = 1.0 / (now - last_time) if last_time else 0.0
    cv2.putText(
        img,
        f"FPS: {fps:.2f}",
        (10, 30),
        cv2.FONT_HERSHEY_SIMPLEX,
        1,
        (0, 255, 0),
        2,
    )
    cv2.imshow(window_name, img)
    quit_pressed = (cv2.waitKey(1) & 0xFF) == ord("q")
    return now, quit_pressed


def _body_tilt_angle_deg(kpts: np.ndarray) -> float:
    shoulders = kpts[[5, 6]].mean(axis=0)
    hips = kpts[[11, 12]].mean(axis=0)
    vector = hips - shoulders
    angle = abs(math.degrees(math.atan2(float(vector[0]), float(vector[1]) + 1e-9)))
    return angle


def _height_ratio(kpts: np.ndarray, box_h: float) -> float:
    head = kpts[0]
    ankles = kpts[[15, 16]].mean(axis=0)
    head_to_ankle = float(np.linalg.norm(ankles - head))
    return head_to_ankle / max(float(box_h), 1e-6)


def _compute_features(kpts: np.ndarray, xyxy: np.ndarray) -> Dict[str, float]:
    x1, y1, x2, y2 = map(float, xyxy)
    width, height = x2 - x1, y2 - y1
    ratio = width / max(height, 1e-6)
    angle = _body_tilt_angle_deg(kpts)
    rel_h = _height_ratio(kpts, height)
    return {"angle": angle, "ratio": ratio, "rel_h": rel_h, "w": width, "h": height}


def _flatten_feature_window(feature_window: List[List[float]]) -> Dict[str, float]:
    payload: Dict[str, float] = {}
    for idx, vec in enumerate(feature_window, start=1):
        angle, ratio, h_ratio = vec
        payload[f"a{idx}"] = float(angle)
        payload[f"r{idx}"] = float(ratio)
        payload[f"h{idx}"] = float(h_ratio)
    return payload


class StreamWorker:
    """Run YOLO pose tracking for a single RTSP stream."""

    def __init__(self, app_config: AppConfig, stream: IPCStreamConfig) -> None:
        self.app_config = app_config
        self.stream = stream
        self._stop_event = threading.Event()
        self._thread: threading.Thread | None = None
        self._window_name = f"Fall-Detection-{stream.ip_address}"

    def start(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        LOGGER.info("Starting stream worker for %s", self.stream.stream_url)
        self._stop_event.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._stop_event.set()
        if self._thread:
            self._thread.join(timeout=2.0)
        if self.app_config.show_windows:
            cv2.destroyWindow(self._window_name)
        LOGGER.info("Stopped stream worker for %s", self.stream.stream_url)

    def update_stream(self, stream: IPCStreamConfig) -> None:
        """Update metadata without restarting."""
        self.stream = stream

    def _run(self) -> None:
        timeout_us = int(self.app_config.stream_connect_timeout * 1_000_000)
        desired_options = f"rtsp_transport;tcp|stimeout;{timeout_us}"
        os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = desired_options

        try:
            model = YOLO(self.app_config.model_path)
        except Exception as exc:
            LOGGER.exception("Failed to load YOLO model for %s: %s", self.stream.stream_url, exc)
            return

        frame_delay = 1.0 / self.app_config.max_fps if self.app_config.max_fps > 0 else 0.0

        while not self._stop_event.is_set():
            if not self._probe_stream(self.stream.stream_url):
                if self._stop_event.wait(self.app_config.stream_reconnect_delay):
                    break
                continue

            history: Dict[int, Deque[Dict[str, float]]] = defaultdict(lambda: deque(maxlen=20))
            feature_seq: Dict[int, Deque[List[float]]] = defaultdict(
                lambda: deque(maxlen=self.app_config.window_frames)
            )
            window_batch: Deque[Dict[str, float]] = deque()
            last_time = 0.0

            try:
                generator = model.track(
                    source=self.stream.stream_url,
                    stream=True,
                    conf=self.app_config.model_conf,
                    imgsz=self.app_config.model_imgsz,
                    vid_stride=1,
                    verbose=False,
                    persist=True,
                )
            except Exception as exc:
                LOGGER.error("Failed to start tracking for %s: %s", self.stream.stream_url, exc)
                if self._stop_event.wait(self.app_config.stream_reconnect_delay):
                    break
                continue

            try:
                for result in generator:
                    if self._stop_event.is_set():
                        break

                    if self.app_config.show_windows:
                        img = result.plot()
                        last_time, quit_pressed = _show_window(self._window_name, img, last_time)
                        if quit_pressed:
                            self._stop_event.set()
                            break

                    keypoints = result.keypoints
                    boxes = result.boxes
                    if (
                        boxes is None
                        or not hasattr(boxes, "xyxy")
                        or boxes.xyxy is None
                        or getattr(boxes.xyxy, "shape", [0])[0] == 0
                    ):
                        if self.app_config.show_windows:
                            img = getattr(result, "orig_img", None)
                            if img is not None:
                                last_time, _ = _show_window(self._window_name, img, last_time)
                        if frame_delay:
                            time.sleep(frame_delay)
                        continue

                    if (
                        keypoints is None
                        or not hasattr(keypoints, "xy")
                        or keypoints.xy is None
                        or getattr(keypoints.xy, "shape", [0, 0, 0])[0] == 0
                    ):
                        if frame_delay:
                            time.sleep(frame_delay)
                        continue

                    xyxy_all = boxes.xyxy.cpu().numpy() if hasattr(boxes, "xyxy") else []
                    if hasattr(boxes, "id") and boxes.id is not None:
                        ids = boxes.id.cpu().numpy().astype(int)
                    else:
                        ids = np.arange(len(xyxy_all))
                    keypoints_all = keypoints.xy.cpu().numpy() if hasattr(keypoints, "xy") else []

                    for i, pid in enumerate(ids):
                        xyxy = xyxy_all[i]
                        kpt = keypoints_all[i]
                        new_features = _compute_features(kpt, xyxy)
                        previous = history[pid][-1] if history[pid] else None
                        h_ratio = (
                            0.0
                            if previous is None
                            else (new_features["h"] / max(previous.get("h", 1e-6), 1e-6))
                        )
                        new_features["h_ratio"] = float(h_ratio)
                        history[pid].append(new_features)

                        feature_vec = [
                            float(new_features.get("angle", 0.0)),
                            float(new_features.get("ratio", 0.0)),
                            float(new_features.get("h_ratio", 1.0)),
                        ]
                        feature_seq[pid].append(feature_vec)

                        if len(feature_seq[pid]) == self.app_config.window_frames:
                            feature_window = list(feature_seq[pid])
                            window_payload = _flatten_feature_window(feature_window)
                            feature_seq[pid].clear()
                            window_batch.append(window_payload)
                            if len(window_batch) >= self.app_config.window_batch_size:
                                send_windows_to_api(self.app_config, self.stream, list(window_batch))
                                window_batch.clear()

                    if frame_delay:
                        time.sleep(frame_delay)
            except Exception as exc:
                LOGGER.error("Error during tracking loop for %s: %s", self.stream.stream_url, exc)
            finally:
                if hasattr(generator, "close"):
                    with contextlib.suppress(Exception):
                        generator.close()

            if not self._stop_event.is_set():
                LOGGER.warning(
                    "Stream interrupted for %s. Retrying in %.1fs",
                    self.stream.stream_url,
                    self.app_config.stream_reconnect_delay,
                )
                if self._stop_event.wait(self.app_config.stream_reconnect_delay):
                    break

        LOGGER.info("Stream loop exiting for %s", self.stream.stream_url)

    def _probe_stream(self, url: str) -> bool:
        """Return True if a frame can be read within connect timeout."""
        LOGGER.info("Probing RTSP stream %s", url)
        cap = cv2.VideoCapture(url, cv2.CAP_FFMPEG)
        start = time.time()
        success = False
        try:
            while not self._stop_event.is_set():
                if time.time() - start > self.app_config.stream_connect_timeout:
                    LOGGER.error(
                        "RTSP connect timeout (%.1fs) for %s",
                        self.app_config.stream_connect_timeout,
                        url,
                    )
                    break
                ret, _ = cap.read()
                if ret:
                    success = True
                    break
                time.sleep(0.1)
        finally:
            cap.release()
        return success
