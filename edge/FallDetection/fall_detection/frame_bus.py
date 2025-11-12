"""Thread-safe registry for latest frames emitted by stream workers."""

from __future__ import annotations

import threading
import time
from dataclasses import dataclass
from typing import Dict, Iterable, List, Tuple

import numpy as np


def _unix_time_ms() -> int:
    """Return the current Unix timestamp in milliseconds."""

    return time.time_ns() // 1_000_000


@dataclass(frozen=True)
class FramePayload:
    """Container for the latest annotated frame of a stream."""

    stream_id: str
    label: str
    frame: np.ndarray
    timestamp_ms: int


class FrameHub:
    """Maintain the newest frame for each stream URL for downstream consumers."""

    def __init__(self) -> None:
        self._frames: Dict[str, FramePayload] = {}
        self._lock = threading.Lock()

    def publish(self, stream_id: str, label: str, frame: np.ndarray) -> None:
        """Store (or update) the latest frame for the given stream."""

        frame_u8 = np.ascontiguousarray(frame, dtype=np.uint8)
        payload = FramePayload(
            stream_id=stream_id,
            label=label,
            frame=frame_u8,
            timestamp_ms=_unix_time_ms(),
        )
        with self._lock:
            self._frames[stream_id] = payload

    def remove(self, stream_id: str) -> None:
        """Drop the stored frame for a stream that has stopped."""

        with self._lock:
            self._frames.pop(stream_id, None)

    def snapshot(self) -> List[FramePayload]:
        """Return a shallow copy of the current frames for composition."""

        with self._lock:
            return sorted(self._frames.values(), key=lambda payload: payload.stream_id)

    def items(self) -> Iterable[Tuple[str, FramePayload]]:
        """Expose iterator primarily for debugging."""

        with self._lock:
            return list(self._frames.items())
