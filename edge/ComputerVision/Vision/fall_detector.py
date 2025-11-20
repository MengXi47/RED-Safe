"""跌倒偵測啟發式相關工具。"""

from __future__ import annotations

import math
from typing import Dict, NamedTuple, Optional, Tuple

import numpy as np


def infer_labels(
    tilt_angle: float,
    height_ratio_value: float,
    angular_velocity: float,
    box_rate_per_s: float,
) -> tuple[int, float]:
    """依啟發式規則推論跌倒信心值。"""

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


def body_tilt_angle_deg(kpts: np.ndarray) -> float:
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


def body_tilt_angular_velocity_deg(
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


def box_height_change_rate(
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


def height_ratio(kpts: np.ndarray, box_h: float) -> float:
    """回傳頭至腳踝距離與框高的比值。"""

    head = None if np.all(np.isclose(kpts[0], 0.0)) else kpts[0]
    ankles = kpts[[15, 16]]
    ankle_mask = ~np.all(np.isclose(ankles, 0.0), axis=1)
    ankle = ankles[ankle_mask].mean(axis=0) if np.any(ankle_mask) else None

    if head is None or ankle is None:
        return float("nan")

    head_ankle = float(np.linalg.norm(ankle - head))
    return round(head_ankle / max(float(box_h), 1e-6), 3)


class FallDetectionResult(NamedTuple):
    """單一次跌倒判斷的結果封裝。"""

    label_weak: int
    log_line: str


class FallDetectionTracker:
    """跨影格追蹤個別人員的跌倒判斷資訊。"""

    def __init__(self) -> None:
        self._last_box_by_id: Dict[int, Tuple[float, int]] = {}
        self._last_tilt_by_id: Dict[int, Tuple[float, int]] = {}

    def evaluate(
        self,
        pid: int,
        height: float,
        keypoints: np.ndarray,
        timestamp_ms: int,
    ) -> FallDetectionResult:
        """計算啟發式指標並回傳弱判斷標籤與除錯文字。"""

        prev_box = self._last_box_by_id.get(pid)
        if prev_box is None:
            prev_height = None
            prev_box_ts = None
        else:
            prev_height, prev_box_ts = prev_box

        scale_ratio, scale_rate = box_height_change_rate(
            height,
            timestamp_ms,
            prev_height,
            prev_box_ts,
        )

        tilt_angle = body_tilt_angle_deg(keypoints)
        prev_tilt = self._last_tilt_by_id.get(pid)
        if prev_tilt is None:
            prev_angle = None
            prev_tilt_ts = None
        else:
            prev_angle, prev_tilt_ts = prev_tilt

        tilt_velocity = body_tilt_angular_velocity_deg(
            tilt_angle,
            timestamp_ms,
            prev_angle,
            prev_tilt_ts,
        )

        if math.isnan(tilt_angle):
            self._last_tilt_by_id.pop(pid, None)
        else:
            self._last_tilt_by_id[pid] = (
                tilt_angle,
                timestamp_ms,
            )

        height_r = height_ratio(keypoints, height)
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

        self._last_box_by_id[pid] = (height, timestamp_ms)
        return FallDetectionResult(
            label_weak=label_weak,
            log_line=", ".join(debug_fields),
        )
