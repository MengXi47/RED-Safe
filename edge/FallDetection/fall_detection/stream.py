"""跌倒偵測串流處理流程。"""

from __future__ import annotations

import contextlib
import logging
import math
import os
import threading
import time
from datetime import datetime, timezone
from typing import Dict, Optional, Tuple, TYPE_CHECKING

import cv2
import numpy as np
from ultralytics import YOLO

from .config import AppConfig
from .db import IPCStreamConfig
from .event_reporter import FallEventReporter
from .frame_bus import FrameHub

if TYPE_CHECKING:
    from ultralytics.engine.results import Results
LOGGER = logging.getLogger(__name__)


def _unix_time_ms() -> int:
    """回傳目前的 Unix 毫秒時間戳。"""

    return time.time_ns() // 1_000_000


def infer_labels(
    tilt_angle: float,
    height_ratio_value: float,
    angular_velocity: float,
    box_rate_per_s: float,
) -> tuple[int, float]:
    """沿用除錯腳本的啟發式規則，推論跌倒信心值。"""

    if (
        math.isnan(tilt_angle)
        or math.isnan(height_ratio_value)
        or math.isnan(angular_velocity)
        or math.isnan(box_rate_per_s)
    ):
        return 0, 0.0

    base_condition = (
        tilt_angle > 55.0
        and height_ratio_value > 1.1
        and angular_velocity > 80
        and box_rate_per_s < -0.5
    )
    f1 = angular_velocity > 100 and box_rate_per_s < -0.2
    f2 = angular_velocity > 170
    weak = int(base_condition or f1 or f2)
    strong = float(weak)
    return weak, strong


def _mean_valid(points: np.ndarray) -> Optional[np.ndarray]:
    """取得有效關節點的平均值（忽略座標為零者）。"""

    mask = ~np.all(np.isclose(points, 0.0), axis=1)
    if not np.any(mask):
        return None
    return points[mask].mean(axis=0)


def _body_tilt_angle_deg(kpts: np.ndarray) -> float:
    """計算肩腰向量相對垂直軸的傾角。"""

    shoulders = _mean_valid(kpts[[5, 6]])
    hips = _mean_valid(kpts[[11, 12]])
    if shoulders is None or hips is None:
        return float("nan")

    vector = hips - shoulders
    angle = abs(
        math.degrees(math.atan2(float(vector[0]), float(vector[1]) + 1e-9))
    )
    return round(angle, 3)


def _body_tilt_angular_velocity_deg(
    current_angle_deg: float,
    current_timestamp_ms: int,
    previous_angle_deg: Optional[float] = None,
    previous_timestamp_ms: Optional[int] = None,
) -> float:
    """計算兩幀之間的角速度（度/秒）。"""

    if math.isnan(current_angle_deg):
        return float("nan")
    if (
        previous_angle_deg is None
        or previous_timestamp_ms is None
        or math.isnan(previous_angle_deg)
    ):
        return 0.0

    dt_ms = current_timestamp_ms - previous_timestamp_ms
    if dt_ms <= 0:
        return 0.0
    velocity = abs((current_angle_deg - previous_angle_deg) / (dt_ms / 1000.0))
    return round(velocity, 3)


def _box_height_change_rate(
    current_height: float,
    current_timestamp_ms: int,
    previous_height: Optional[float] = None,
    previous_timestamp_ms: Optional[int] = None,
) -> tuple[float, float]:
    """回傳框高比例與每秒變化率。"""

    if (
        previous_height is None
        or previous_timestamp_ms is None
        or previous_height <= 0.0
    ):
        return 1.0, 0.0

    dt_ms = current_timestamp_ms - previous_timestamp_ms
    if dt_ms <= 0:
        return 1.0, 0.0

    dt = dt_ms / 1000.0
    ratio = current_height / max(previous_height, 1e-6)
    rate = (ratio - 1.0) / dt
    return round(ratio, 3), round(rate, 3)


def _height_ratio(kpts: np.ndarray, box_h: float) -> float:
    """回傳頭至腳踝距離與框高的比值。"""

    head = None if np.all(np.isclose(kpts[0], 0.0)) else kpts[0]
    ankles = kpts[[15, 16]]
    ankle_mask = ~np.all(np.isclose(ankles, 0.0), axis=1)
    ankle = ankles[ankle_mask].mean(axis=0) if np.any(ankle_mask) else None

    if head is None or ankle is None:
        return float("nan")

    head_ankle = float(np.linalg.norm(ankle - head))
    return round(head_ankle / max(float(box_h), 1e-6), 3)


class StreamWorker:
    """針對單一 RTSP 串流執行 YOLO 姿態追蹤。"""

    def __init__(
        self,
        app_config: AppConfig,
        stream: IPCStreamConfig,
        frame_hub: FrameHub,
    ) -> None:
        self.app_config = app_config
        self.stream = stream
        self._stop_event = threading.Event()
        self._thread: threading.Thread | None = None
        self._frame_hub = frame_hub
        self._fall_overlay_until = 0.0
        self._event_reporter = FallEventReporter(app_config)
        self._last_report_ts = 0.0

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
        LOGGER.info("Stopped stream worker for %s", self.stream.stream_url)

    def update_stream(self, stream: IPCStreamConfig) -> None:
        """在不重啟執行緒的情況下更新串流資訊。"""
        if stream.stream_url != self.stream.stream_url:
            self._frame_hub.remove(self.stream.stream_url)
        self.stream = stream

    def _run(self) -> None:
        timeout_us = int(self.app_config.stream_connect_timeout * 1_000_000)
        desired_options = f"rtsp_transport;tcp|stimeout;{timeout_us}"
        os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = desired_options

        try:
            model = YOLO(self.app_config.model_path)
        except Exception as exc:
            LOGGER.exception(
                "Failed to load YOLO model for %s: %s",
                self.stream.stream_url,
                exc,
            )
            return

        frame_delay = (
            1.0 / self.app_config.max_fps
            if self.app_config.max_fps > 0
            else 0.0
        )

        while not self._stop_event.is_set():
            if not self._probe_stream(self.stream.stream_url):
                if self._stop_event.wait(
                    self.app_config.stream_reconnect_delay
                ):
                    break
                continue

            last_box_by_id: Dict[int, Tuple[float, int]] = {}
            last_tilt_by_id: Dict[int, Tuple[float, int]] = {}

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
                LOGGER.error(
                    "Failed to start tracking for %s: %s",
                    self.stream.stream_url,
                    exc,
                )
                if self._stop_event.wait(
                    self.app_config.stream_reconnect_delay
                ):
                    break
                continue

            try:
                for result in generator:
                    if self._stop_event.is_set():
                        break

                    keypoints = result.keypoints
                    boxes = result.boxes

                    has_valid_boxes = (
                        boxes is not None
                        and hasattr(boxes, "xyxy")
                        and boxes.xyxy is not None
                        and getattr(boxes.xyxy, "shape", [0])[0] > 0
                    )
                    has_valid_kpts = (
                        keypoints is not None
                        and hasattr(keypoints, "xy")
                        and keypoints.xy is not None
                        and getattr(keypoints.xy, "shape", [0, 0, 0])[0] > 0
                    )

                    if has_valid_boxes and has_valid_kpts:
                        xyxy_all = (
                            boxes.xyxy.cpu().numpy()
                            if hasattr(boxes, "xyxy")
                            else np.empty((0, 4))
                        )
                        if hasattr(boxes, "id") and boxes.id is not None:
                            ids = boxes.id.cpu().numpy().astype(int)
                        else:
                            ids = np.arange(len(xyxy_all))
                        keypoints_all = (
                            keypoints.xy.cpu().numpy()
                            if hasattr(keypoints, "xy")
                            else np.empty((0, 17, 2))
                        )

                        for i, pid in enumerate(ids):
                            if i >= len(xyxy_all) or i >= len(keypoints_all):
                                continue
                            xyxy = xyxy_all[i]
                            kpt = keypoints_all[i]
                            _, y1, _, y2 = map(float, xyxy)
                            height = y2 - y1

                            timestamp_ms = _unix_time_ms()
                            prev_box = last_box_by_id.get(pid)
                            if prev_box is None:
                                prev_height = None
                                prev_box_ts = None
                            else:
                                prev_height, prev_box_ts = prev_box

                            scale_ratio, scale_rate = _box_height_change_rate(
                                height,
                                timestamp_ms,
                                prev_height,
                                prev_box_ts,
                            )

                            tilt_angle = _body_tilt_angle_deg(kpt)
                            prev_tilt = last_tilt_by_id.get(pid)
                            if prev_tilt is None:
                                prev_angle = None
                                prev_tilt_ts = None
                            else:
                                prev_angle, prev_tilt_ts = prev_tilt

                            tilt_velocity = _body_tilt_angular_velocity_deg(
                                tilt_angle,
                                timestamp_ms,
                                prev_angle,
                                prev_tilt_ts,
                            )

                            if math.isnan(tilt_angle):
                                last_tilt_by_id.pop(pid, None)
                            else:
                                last_tilt_by_id[pid] = (
                                    tilt_angle,
                                    timestamp_ms,
                                )

                            height_r = _height_ratio(kpt, height)
                            label_weak, _ = infer_labels(
                                tilt_angle,
                                height_r,
                                tilt_velocity,
                                scale_rate,
                            )

                            debug_fields = [
                                f"ID {pid}",
                                f"tilt={tilt_angle} deg",
                                f"omega={tilt_velocity} deg/s",
                                f"box_ratio={scale_ratio}",
                                f"box_rate={scale_rate} 1/s",
                                f"height_ratio={height_r}",
                                f"label_weak={label_weak}",
                            ]

                            log_line = ", ".join(debug_fields)
                            if self.app_config.debug:
                                print(log_line)
                            if label_weak:
                                self._report_fall_event(result)
                                LOGGER.warning(
                                    "Fall candidate on %s (pid=%s): %s",
                                    self.stream.stream_url,
                                    pid,
                                    log_line,
                                )
                                self._fall_overlay_until = time.time() + 5.0
                            else:
                                LOGGER.debug(
                                    "Stream %s pid=%s metrics: %s",
                                    self.stream.stream_url,
                                    pid,
                                    log_line,
                                )

                            last_box_by_id[pid] = (height, timestamp_ms)

                    self._publish_frame(result)

                    sleep_delay = frame_delay if frame_delay > 0 else 1 / 3.7
                    if sleep_delay > 0:
                        time.sleep(sleep_delay)
            except Exception as exc:
                LOGGER.error(
                    "Error during tracking loop for %s: %s",
                    self.stream.stream_url,
                    exc,
                )
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
                if self._stop_event.wait(
                    self.app_config.stream_reconnect_delay
                ):
                    break

        self._frame_hub.remove(self.stream.stream_url)
        LOGGER.info("Stream loop exiting for %s", self.stream.stream_url)

    def _probe_stream(self, url: str) -> bool:
        """於連線逾時前讀到影格則回傳 True。"""
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

    def _publish_frame(self, result: "Results") -> None:
        """送出帶偵測匡線的畫面給匯流排，供 WebRTC 使用。"""

        try:
            plotted = result.plot()
        except Exception as exc:  # pragma: no cover - best effort
            LOGGER.debug(
                "Failed to render frame for %s: %s", self.stream.stream_url, exc
            )
            return

        if plotted.dtype != np.uint8:
            plotted = np.clip(plotted, 0, 255).astype(np.uint8)

        if time.time() < self._fall_overlay_until:
            overlay = plotted.copy()
            height, width = overlay.shape[:2]
            box_height = int(height * 0.15)
            cv2.rectangle(overlay, (0, 0), (width, box_height), (0, 0, 255), -1)
            cv2.putText(
                overlay,
                "Fall!!!!!!!!",
                (int(width * 0.05), int(box_height * 0.7)),
                cv2.FONT_HERSHEY_SIMPLEX,
                2.0,
                (255, 255, 255),
                4,
                lineType=cv2.LINE_AA,
            )
            plotted = cv2.addWeighted(plotted, 0.5, overlay, 0.5, 0)

        label = (
            self.stream.custom_name
            or self.stream.ipc_name
            or self.stream.ip_address
            or self.stream.stream_url
        )
        self._frame_hub.publish(self.stream.stream_url, label, plotted)

    def _report_fall_event(self, result: "Results") -> None:
        """於偵測到跌倒時將畫面與資訊送往雲端 API。"""

        if not self.app_config.fall_event_enabled:
            return

        now = time.time()
        if now - self._last_report_ts < self.app_config.fall_event_cooldown:
            return

        snapshot = self._encode_snapshot(result)
        event_time_iso = datetime.now(timezone.utc).isoformat()
        self._event_reporter.report(self.stream, snapshot, event_time_iso)
        self._last_report_ts = now

    def _encode_snapshot(self, result: "Results") -> bytes | None:
        """將當前原始畫面轉為 JPEG bytes，失敗時回傳 None。"""

        frame = getattr(result, "orig_img", None)
        if frame is None:
            return None

        success, buffer = cv2.imencode(".jpg", frame)
        if not success:
            return None
        return buffer.tobytes()
