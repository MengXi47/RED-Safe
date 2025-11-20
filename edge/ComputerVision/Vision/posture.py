"""姿態分類工具（站/坐/睡）。"""

from __future__ import annotations

import math
from typing import Literal

import numpy as np

PostureLabel = Literal["stand", "sit", "sleep", "not_stand"]

# ---- 阈值與參數（沿用 test.py 演算法）----
STAND_TILT_THRESHOLD = 25.0
STAND_HEIGHT_RATIO_THRESHOLD = 0.7
STAND_ALIGNMENT_THRESHOLD = 0.12

SIT_LEG_VERTICAL_RATIO_MAX = 0.25
SIT_THIGH_HORIZONTAL_RATIO_MAX = 0.35
SIT_TORSO_VERTICAL_RATIO_MAX = 0.6
SIT_KNEE_COS_ABS_MAX = 0.4

SLEEP_COLLINEARITY_MAX = 0.18
# ---- /參數 ----


def _mean_valid(points: np.ndarray):
    mask = ~np.all(np.isclose(points, 0.0), axis=1)
    if not np.any(mask):
        return None
    return points[mask].mean(axis=0)


def _body_tilt_angle_deg(kpts: np.ndarray) -> float:
    shoulders = _mean_valid(kpts[[5, 6]])
    hips = _mean_valid(kpts[[11, 12]])
    if shoulders is None or hips is None:
        return float("nan")

    v = hips - shoulders
    angle = abs(math.degrees(math.atan2(float(v[0]), float(v[1]) + 1e-9)))
    return round(angle, 3)


def _height_ratio(kpts: np.ndarray, box_h: float) -> float:
    if box_h <= 0:
        return float("nan")

    head = None if np.all(np.isclose(kpts[0], 0.0)) else kpts[0]
    if head is None:
        return float("nan")

    ankles = kpts[[15, 16]]
    ankle_mask = ~np.all(np.isclose(ankles, 0.0), axis=1)
    if np.any(ankle_mask):
        ankle = ankles[ankle_mask].mean(axis=0)
        bottom_y = float(ankle[1])
    else:
        candidate_idx = [15, 16, 13, 14, 11, 12]
        ys = []
        for idx in candidate_idx:
            pt = kpts[idx]
            if not np.all(np.isclose(pt, 0.0)):
                ys.append(float(pt[1]))
        if not ys:
            return float("nan")
        bottom_y = max(ys)

    head_y = float(head[1])
    dist = abs(bottom_y - head_y)
    return round(dist / max(float(box_h), 1e-6), 3)


def _alignment_deviation(kpts: np.ndarray, ref_scale: float) -> float:
    pts = []
    for pair in [(5, 6), (11, 12), (13, 14), (15, 16)]:
        p = _mean_valid(kpts[list(pair)])
        if p is not None:
            pts.append(float(p[0]))
    if len(pts) < 3 or ref_scale <= 0:
        return float("nan")
    xs = np.array(pts)
    mean_x = xs.mean()
    mad = np.abs(xs - mean_x).mean()
    return round(mad / ref_scale, 3)


def _sitting_geometry_scores(kpts: np.ndarray):
    ankle = _mean_valid(kpts[[15, 16]])
    knee = _mean_valid(kpts[[13, 14]])
    hip = _mean_valid(kpts[[11, 12]])
    shoulder = _mean_valid(kpts[[5, 6]])
    if ankle is None or knee is None or hip is None or shoulder is None:
        return float("nan"), float("nan"), float("nan"), float("nan")

    ax, ay = float(ankle[0]), float(ankle[1])
    kx, ky = float(knee[0]), float(knee[1])
    hx, hy = float(hip[0]), float(hip[1])
    sx, sy = float(shoulder[0]), float(shoulder[1])

    Lx, Ly = ax - kx, ay - ky
    Tx, Ty = hx - kx, hy - ky
    Ux, Uy = sx - hx, sy - hy

    leg_len = math.hypot(Lx, Ly)
    thigh_len = math.hypot(Tx, Ty)
    torso_len = math.hypot(Ux, Uy)
    if leg_len <= 1e-6 or thigh_len <= 1e-6 or torso_len <= 1e-6:
        return float("nan"), float("nan"), float("nan"), float("nan")

    leg_vert_ratio = abs(ax - kx) / leg_len
    thigh_horiz_ratio = abs(hy - ky) / thigh_len
    torso_vert_ratio = abs(sx - hx) / torso_len
    knee_cos = (Lx * Tx + Ly * Ty) / (leg_len * thigh_len)

    return (
        round(leg_vert_ratio, 3),
        round(thigh_horiz_ratio, 3),
        round(torso_vert_ratio, 3),
        round(knee_cos, 3),
    )


def _sleep_collinearity_score(kpts: np.ndarray) -> float:
    shoulder = _mean_valid(kpts[[5, 6]])
    hip = _mean_valid(kpts[[11, 12]])
    knee = _mean_valid(kpts[[13, 14]])
    ankle = _mean_valid(kpts[[15, 16]])
    if shoulder is None or hip is None or knee is None or ankle is None:
        return float("nan")

    S = np.array(shoulder, dtype=float)
    H = np.array(hip, dtype=float)
    K = np.array(knee, dtype=float)
    A = np.array(ankle, dtype=float)

    v1 = H - S
    v2 = K - H
    v3 = A - K

    def _cross_ratio(a, b):
        cross = abs(a[0] * b[1] - a[1] * b[0])
        denom = (math.hypot(a[0], a[1]) * math.hypot(b[0], b[1]) + 1e-9)
        return cross / denom

    r12 = _cross_ratio(v1, v2)
    r23 = _cross_ratio(v2, v3)
    return round((r12 + r23) * 0.5, 3)


def classify_posture(kpts: np.ndarray, box_height: float) -> PostureLabel:
    """回傳姿態標籤：stand / sit / sleep / not_stand。"""
    tilt = _body_tilt_angle_deg(kpts)
    height_r = _height_ratio(kpts, box_height)
    align_dev = _alignment_deviation(kpts, box_height)
    is_stand = (
        not math.isnan(tilt)
        and not math.isnan(height_r)
        and not math.isnan(align_dev)
        and tilt < STAND_TILT_THRESHOLD
        and height_r > STAND_HEIGHT_RATIO_THRESHOLD
        and align_dev < STAND_ALIGNMENT_THRESHOLD
    )
    if is_stand:
        return "stand"

    leg_vert_ratio, thigh_horiz_ratio, torso_vert_ratio, knee_cos = _sitting_geometry_scores(
        kpts
    )
    if not any(math.isnan(v) for v in (leg_vert_ratio, thigh_horiz_ratio, torso_vert_ratio, knee_cos)):
        is_sit = (
            leg_vert_ratio < SIT_LEG_VERTICAL_RATIO_MAX
            and thigh_horiz_ratio < SIT_THIGH_HORIZONTAL_RATIO_MAX
            and torso_vert_ratio < SIT_TORSO_VERTICAL_RATIO_MAX
            and abs(knee_cos) < SIT_KNEE_COS_ABS_MAX
        )
        if is_sit:
            return "sit"

    col_score = _sleep_collinearity_score(kpts)
    if not math.isnan(col_score) and col_score < SLEEP_COLLINEARITY_MAX:
        return "sleep"

    return "not_stand"
