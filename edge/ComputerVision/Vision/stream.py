"""跌倒偵測串流處理流程。"""

from __future__ import annotations

import contextlib
import logging
import os
import threading
import time
from datetime import datetime, timezone
from typing import Iterator, Optional, TYPE_CHECKING

import cv2
import numpy as np
from ultralytics import YOLO

from .config import AppConfig
from .db import DatabaseClient, IPCStreamConfig
from .event_reporter import FallEventReporter
from .frame_bus import FrameHub
from .bednight import BedExitDetector
from .fall_detector import FallDetectionTracker
from .inactivity import InactivityDetector
from .posture import classify_posture

if TYPE_CHECKING:
    from ultralytics.engine.results import Results
LOGGER = logging.getLogger(__name__)


def _unix_time_ms() -> int:
    """回傳目前的 Unix 毫秒時間戳。"""

    return time.time_ns() // 1_000_000


class StreamWorker:
    """針對單一 RTSP 串流執行 YOLO 姿態追蹤。"""

    def __init__(
        self,
        app_config: AppConfig,
        stream: IPCStreamConfig,
        frame_hub: FrameHub,
        db_client: DatabaseClient,
    ) -> None:
        self.app_config = app_config
        self.stream = stream
        self._stop_event = threading.Event()
        self._thread: threading.Thread | None = None
        self._generator_lock = threading.Lock()
        self._active_generator: Optional[Iterator["Results"]] = None
        self._frame_hub = frame_hub
        self._model: YOLO | None = None
        self._fall_overlay_until = 0.0
        self._bed_overlay_until = 0.0
        self._inactivity_overlay_until = 0.0
        self._event_reporter = FallEventReporter(app_config)
        self._last_report_ts = 0.0
        self._last_posture_labels: dict[int, str] = {}
        self._db_client = db_client
        self._bed_exit_detector = BedExitDetector(
            ip_address=self.stream.ip_address,
            roi_loader=self._load_bed_roi_zone,
            refresh_seconds=1.0,
        )
        self._inactivity_detector = InactivityDetector(
            ip_address=self.stream.ip_address,
            policy_loader=self._load_inactivity_policy,
            refresh_seconds=3.0,
        )

    def start(self) -> None:
        """啟動專屬執行緒，建立串流推論迴圈。"""
        if self._thread and self._thread.is_alive():
            return
        LOGGER.info("Starting stream worker for %s", self.stream.stream_url)
        self._stop_event.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        """將停止事件設為 True 並確保資源被釋放。"""
        self._stop_event.set()
        self._close_active_generator()
        if self._thread:
            self._thread.join(timeout=2.0)
        LOGGER.info("Stopped stream worker for %s", self.stream.stream_url)

    def update_stream(self, stream: IPCStreamConfig) -> None:
        """在不重啟執行緒的情況下更新串流資訊。"""
        prev_stream = self.stream
        if stream.stream_url != self.stream.stream_url:
            self._frame_hub.remove(self.stream.stream_url)
        self.stream = stream
        if prev_stream and prev_stream.fall_detection_enabled != stream.fall_detection_enabled:
            LOGGER.info(
                "Fall detection policy updated for %s: enabled=%s",
                self.stream.stream_url,
                self.stream.fall_detection_enabled,
            )
        self._bed_exit_detector.update_ip(stream.ip_address)
        self._inactivity_detector.update_ip(stream.ip_address)

    def _load_bed_roi_zone(self, ip_address: str):
        """取得單一 IPC 的床位 ROI 設定。"""

        try:
            return self._db_client.fetch_bed_roi_zone(ip_address)
        except Exception as exc:
            LOGGER.debug("Failed to fetch bed ROI zones: %s", exc)
            return None

    def _load_inactivity_policy(self, ip_address: str):
        """取得單一 IPC 的靜止偵測策略。"""

        try:
            return self._db_client.fetch_inactivity_policy(ip_address)
        except Exception as exc:
            LOGGER.debug("Failed to fetch inactivity policy: %s", exc)
            return None

    def _close_active_generator(self) -> None:
        """關閉目前的 YOLO 追蹤產生器，避免持續抓取影格。"""
        with self._generator_lock:
            generator = self._active_generator
            self._active_generator = None
        if generator and hasattr(generator, "close"):
            with contextlib.suppress(Exception):
                generator.close()
        self._close_predictor_dataset()

    def _close_predictor_dataset(self) -> None:
        """釋放 Ultralytics 內部的資料來源，確實斷開 RTSP。"""
        model = self._model
        if not model:
            return
        predictor = getattr(model, "predictor", None)
        dataset = getattr(predictor, "dataset", None)
        if dataset and hasattr(dataset, "close"):
            with contextlib.suppress(Exception):
                dataset.close()
            try:
                predictor.dataset = None  # type: ignore[attr-defined]
            except Exception:
                pass

    def _run(self) -> None:
        """主要執行迴圈，負責連線、推論與重試邏輯。"""
        timeout_us = int(self.app_config.stream_connect_timeout * 1_000_000)
        desired_options = f"rtsp_transport;tcp|stimeout;{timeout_us}"
        os.environ["OPENCV_FFMPEG_CAPTURE_OPTIONS"] = desired_options

        try:
            model = YOLO(self.app_config.model_path)
            self._model = model
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

            fall_tracker = FallDetectionTracker()

            generator = None
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

            with self._generator_lock:
                self._active_generator = generator

            try:
                for result in generator:
                    if self._stop_event.is_set():
                        break

                    keypoints = result.keypoints
                    boxes = result.boxes
                    self._last_posture_labels = {}

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
                            x1, y1, x2, y2 = map(float, xyxy)
                            height = y2 - y1
                            now_ms = _unix_time_ms()
                            int_pid = int(pid)

                            if self._inactivity_detector.is_enabled():
                                posture_label = classify_posture(kpt, height)
                                self._last_posture_labels[int_pid] = posture_label

                            bed_result = self._bed_exit_detector.evaluate(
                                bbox_xyxy=(x1, y1, x2, y2),
                                frame_shape=getattr(result, "orig_shape", None)
                                or getattr(boxes, "orig_shape", None)
                                or (),
                            )
                            if bed_result:
                                if bed_result.triggered:
                                    self._log_edge_event(
                                        "夜間離床",
                                        {
                                            "ip_address": self.stream.ip_address,
                                            "stream_url": self.stream.stream_url,
                                            "pid": int(pid),
                                            "outside_ratio": bed_result.outside_ratio,
                                            "inside_ratio": bed_result.intersection_ratio,
                                            "event_time": datetime.now(
                                                timezone.utc
                                            ).isoformat(),
                                        },
                                    )
                                    # TODO: 發出夜間離床事件
                                    LOGGER.warning(
                                        "Bed exit candidate on %s (pid=%s): %s",
                                        self.stream.stream_url,
                                        pid,
                                        bed_result.log_line,
                                    )
                                    self._bed_overlay_until = time.time() + 5.0
                                elif self.app_config.debug:
                                    LOGGER.debug(
                                        "Bed zone metrics %s pid=%s: %s",
                                        self.stream.stream_url,
                                        pid,
                                        bed_result.log_line,
                                    )

                            inactivity_result = self._inactivity_detector.evaluate(
                                pid=pid,
                                bbox_xyxy=(x1, y1, x2, y2),
                                timestamp_ms=now_ms,
                            )
                            if inactivity_result:
                                LOGGER.debug(
                                    "Inactivity metrics %s pid=%s: %s",
                                    self.stream.stream_url,
                                    pid,
                                    inactivity_result.log_line,
                                )
                                if inactivity_result.triggered:
                                    self._log_edge_event(
                                        "長時間靜止",
                                        {
                                            "ip_address": self.stream.ip_address,
                                            "stream_url": self.stream.stream_url,
                                            "pid": int_pid,
                                            "idle_seconds": inactivity_result.idle_seconds,
                                            "event_time": datetime.now(timezone.utc).isoformat(),
                                        },
                                    )
                                    # TODO: 若需上報雲端 API，於此串接與節流
                                    LOGGER.warning(
                                        "Inactivity triggered on %s (pid=%s): %s",
                                        self.stream.stream_url,
                                        pid,
                                        inactivity_result.log_line,
                                    )
                                    self._inactivity_overlay_until = time.time() + 5.0

                            # 跌倒辨識
                            if self.stream.fall_detection_enabled:
                                detection = fall_tracker.evaluate(
                                    pid,
                                    height,
                                    kpt,
                                    now_ms,
                                )

                                log_line = detection.log_line
                                if self.app_config.debug:
                                    print(log_line)
                                if detection.label_weak:
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

                    # 無論是否偵測都推送畫面
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
                self._close_active_generator()

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
        self._model = None
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

        frame = getattr(result, "orig_img", None)
        if frame is None:
            return

        plotted = frame.copy()

        try:
            boxes = getattr(result, "boxes", None)
            if (
                boxes is not None
                and hasattr(boxes, "xyxy")
                and boxes.xyxy is not None
                and getattr(boxes.xyxy, "shape", [0])[0] > 0
            ):
                xyxy_all = (
                    boxes.xyxy.cpu().numpy()
                    if hasattr(boxes.xyxy, "cpu")
                    else np.asarray(boxes.xyxy)
                )
                cls_all = (
                    boxes.cls.cpu().numpy().astype(int)
                    if hasattr(boxes, "cls") and boxes.cls is not None
                    else None
                )
                ids = (
                    boxes.id.cpu().numpy().astype(int)
                    if hasattr(boxes, "id") and boxes.id is not None
                    else None
                )
                use_posture_color = self._inactivity_detector.is_enabled()

                for idx, xyxy in enumerate(xyxy_all):
                    if cls_all is not None and idx < len(cls_all):
                        if cls_all[idx] != 0:
                            continue
                    x1, y1, x2, y2 = [int(v) for v in xyxy]
                    pid = int(ids[idx]) if ids is not None and idx < len(ids) else idx
                    label = self._last_posture_labels.get(pid) if use_posture_color else None
                    if label == "stand":
                        color = (0, 255, 0)
                    elif label == "sit":
                        color = (255, 0, 0)
                    elif label == "sleep":
                        color = (128, 0, 255)
                    elif label == "not_stand":
                        color = (0, 255, 0)
                    else:
                        color = (0, 255, 0)
                    cv2.rectangle(plotted, (x1, y1), (x2, y2), color, 2)
                    if label:
                        cv2.putText(
                            plotted,
                            label,
                            (x1, max(0, y1 - 10)),
                            cv2.FONT_HERSHEY_SIMPLEX,
                            1.1,
                            color,
                            3,
                            lineType=cv2.LINE_AA,
                        )
        except Exception as exc:  # pragma: no cover - best effort
            LOGGER.debug(
                "Failed to render bounding boxes for %s: %s",
                self.stream.stream_url,
                exc,
            )

        if plotted.dtype != np.uint8:
            plotted = np.clip(plotted, 0, 255).astype(np.uint8)

        now = time.time()
        overlay_messages = []
        overlay_colors = []
        if now < self._fall_overlay_until:
            overlay_messages.append("Fall!")
            overlay_colors.append((0, 0, 255))  # red
        if now < self._bed_overlay_until:
            overlay_messages.append("Bed Exit!")
            overlay_colors.append((0, 165, 255))  # orange
        if now < self._inactivity_overlay_until:
            overlay_messages.append("No Movement!")
            overlay_colors.append((255, 255, 0))  # yellow

        if overlay_messages:
            overlay = plotted.copy()
            height, width = overlay.shape[:2]
            box_height = int(height * 0.15)
            cv2.rectangle(overlay, (0, 0), (width, box_height), (0, 0, 0), -1)
            for idx, msg in enumerate(overlay_messages):
                y = int(box_height * (0.5 + 0.4 * idx))
                color = overlay_colors[idx]
                cv2.putText(
                    overlay,
                    msg,
                    (int(width * 0.05), y),
                    cv2.FONT_HERSHEY_SIMPLEX,
                    2.0,
                    color,
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

        now = time.time()
        if now - self._last_report_ts < self.app_config.fall_event_cooldown:
            return

        event_time_iso = datetime.now(timezone.utc).isoformat()
        self._log_edge_event(
            "跌倒",
            {
                "ip_address": self.stream.ip_address,
                "stream_url": self.stream.stream_url,
                "event_time": event_time_iso,
            },
        )

        if not self.app_config.fall_event_enabled:
            self._last_report_ts = now
            return

        snapshot = self._encode_snapshot(result)
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

    def _log_edge_event(self, event_type: str, payload: dict) -> None:
        """寫入 edge_events 表，失敗時只記錄除錯訊息。"""

        try:
            self._db_client.log_edge_event(event_type, payload)
        except Exception as exc:
            LOGGER.debug("Failed to log event %s: %s", event_type, exc)
