"""Utilities for composing multiple stream frames into a single mosaic frame."""

from __future__ import annotations

import threading
import time
from dataclasses import dataclass
from typing import List, Sequence

import cv2
import numpy as np

from .frame_bus import FrameHub, FramePayload


@dataclass(frozen=True)
class MosaicSettings:
    """Configuration for the composed mosaic output."""

    width: int = 1280
    height: int = 720
    fps: float = 5.0
    max_tiles: int = 4


class MosaicComposer:
    """Background worker that composes a NxN mosaic from the latest frames."""

    def __init__(self, frame_hub: FrameHub, settings: MosaicSettings) -> None:
        self._frame_hub = frame_hub
        self._settings = settings
        self._stop_event = threading.Event()
        self._thread: threading.Thread | None = None
        self._condition = threading.Condition()
        self._latest_frame: np.ndarray = self._blank_canvas("等待串流來源")
        self._latest_timestamp = 0

    def start(self) -> None:
        if self._thread and self._thread.is_alive():
            return
        self._stop_event.clear()
        self._thread = threading.Thread(target=self._run, daemon=True)
        self._thread.start()

    def stop(self) -> None:
        self._stop_event.set()
        with self._condition:
            self._condition.notify_all()
        if self._thread:
            self._thread.join(timeout=2.0)

    def wait_for_frame(
        self, after_timestamp: int | None, timeout: float = 1.5
    ) -> tuple[np.ndarray, int]:
        """Block until a newer frame is available (or timeout)."""

        with self._condition:
            if after_timestamp is None and self._latest_frame is not None:
                return self._latest_frame, self._latest_timestamp

            target_ts = after_timestamp
            self._condition.wait_for(
                lambda: self._stop_event.is_set()
                or self._latest_frame is not None
                and self._latest_timestamp != target_ts,
                timeout=timeout,
            )
            return self._latest_frame, self._latest_timestamp

    def _run(self) -> None:
        fps = max(self._settings.fps, 0.5)
        delay = 1.0 / fps
        while not self._stop_event.is_set():
            frames = self._frame_hub.snapshot()
            composed = self._compose(frames)
            self._publish(composed)
            if delay > 0:
                self._stop_event.wait(delay)

    def _publish(self, frame: np.ndarray) -> None:
        with self._condition:
            self._latest_frame = frame
            self._latest_timestamp = time.time_ns()
            self._condition.notify_all()

    def _compose(self, frames: List[FramePayload]) -> np.ndarray:
        if not frames:
            return self._blank_canvas("Wait IPC connect.")

        trimmed = frames[: self._settings.max_tiles]
        if len(trimmed) == 1:
            single = self._resize(
                trimmed[0].frame, self._settings.width, self._settings.height
            )
            return single
        return self._grid(trimmed)

    def _grid(self, frames: Sequence[FramePayload]) -> np.ndarray:
        grid_size = 2
        tile_w = self._settings.width // grid_size
        tile_h = self._settings.height // grid_size
        tiles: List[np.ndarray] = []
        for idx in range(grid_size * grid_size):
            if idx < len(frames):
                payload = frames[idx]
                tile = self._resize(payload.frame, tile_w, tile_h)
            else:
                tile = self._blank_tile(tile_w, tile_h)
            tiles.append(tile)

        top = np.hstack(tiles[:grid_size])
        bottom = np.hstack(tiles[grid_size:])
        return np.vstack([top, bottom])

    def _resize(self, frame: np.ndarray, width: int, height: int) -> np.ndarray:
        if frame.shape[1] == width and frame.shape[0] == height:
            return frame
        return cv2.resize(frame, (width, height), interpolation=cv2.INTER_AREA)

    def _draw_label(self, frame: np.ndarray, label: str) -> np.ndarray:
        # 保留函式以相容後續需求，目前不在畫面上顯示 IPC 名稱。
        return frame

    def _blank_tile(self, width: int, height: int) -> np.ndarray:
        tile = np.zeros((height, width, 3), dtype=np.uint8)
        text = "No Signal"
        font = cv2.FONT_HERSHEY_SIMPLEX
        font_scale = 0.9
        thickness = 2
        (text_width, text_height), baseline = cv2.getTextSize(
            text, font, font_scale, thickness
        )
        x = (width - text_width) // 2
        y = (height + text_height) // 2
        cv2.putText(
            tile,
            text,
            (x, y),
            font,
            font_scale,
            (80, 80, 80),
            thickness,
            lineType=cv2.LINE_AA,
        )
        return tile

    def _blank_canvas(self, label: str) -> np.ndarray:
        canvas = np.zeros(
            (self._settings.height, self._settings.width, 3),
            dtype=np.uint8,
        )
        cv2.putText(
            canvas,
            label,
            (int(self._settings.width * 0.2), int(self._settings.height * 0.5)),
            cv2.FONT_HERSHEY_SIMPLEX,
            1.0,
            (140, 140, 140),
            2,
            lineType=cv2.LINE_AA,
        )
        return canvas
