"""夜間離床相關工具函式。"""

from __future__ import annotations

import math
import threading
import time as time_module
from dataclasses import dataclass
from datetime import datetime, time
import logging
from typing import Callable, List, Optional, Sequence, Tuple

from .config import DatabaseConfig

try:
    import psycopg
except Exception:  # pragma: no cover - optional at import time
    psycopg = None  # type: ignore


LOGGER = logging.getLogger(__name__)


def sort_polygon_points(points: Sequence[Tuple[float, float]]) -> List[Tuple[float, float]]:
    """以質心為基準依極角排序，確保多邊形點順序一致。"""

    if not points:
        return []
    cx = sum(x for x, _ in points) / len(points)
    cy = sum(y for _, y in points) / len(points)
    return sorted(points, key=lambda p: math.atan2(p[1] - cy, p[0] - cx))


def _polygon_area(points: Sequence[Tuple[float, float]]) -> float:
    if len(points) < 3:
        return 0.0
    area = 0.0
    for i, (x1, y1) in enumerate(points):
        x2, y2 = points[(i + 1) % len(points)]
        area += x1 * y2 - x2 * y1
    return abs(area) / 2.0


def _clip_polygon_with_rect(
    polygon: Sequence[Tuple[float, float]],
    x_min: float,
    y_min: float,
    x_max: float,
    y_max: float,
) -> List[Tuple[float, float]]:
    """Sutherland–Hodgman 以矩形裁剪多邊形。"""

    def clip_edge(
        pts: Sequence[Tuple[float, float]],
        inside: Callable[[Tuple[float, float]], bool],
        intersect: Callable[[Tuple[float, float], Tuple[float, float]], Tuple[float, float]],
    ) -> List[Tuple[float, float]]:
        if not pts:
            return []
        output: List[Tuple[float, float]] = []
        s = pts[-1]
        for e in pts:
            if inside(e):
                if inside(s):
                    output.append(e)
                else:
                    output.append(intersect(s, e))
                    output.append(e)
            elif inside(s):
                output.append(intersect(s, e))
            s = e
        return output

    def intersect_x(s: Tuple[float, float], e: Tuple[float, float], x: float) -> Tuple[float, float]:
        if math.isclose(s[0], e[0]):
            return x, s[1]
        t = (x - s[0]) / (e[0] - s[0])
        y = s[1] + t * (e[1] - s[1])
        return x, y

    def intersect_y(s: Tuple[float, float], e: Tuple[float, float], y: float) -> Tuple[float, float]:
        if math.isclose(s[1], e[1]):
            return s[0], y
        t = (y - s[1]) / (e[1] - s[1])
        x = s[0] + t * (e[0] - s[0])
        return x, y

    poly = clip_edge(
        polygon,
        inside=lambda p: p[0] >= x_min,
        intersect=lambda s, e: intersect_x(s, e, x_min),
    )
    poly = clip_edge(
        poly,
        inside=lambda p: p[0] <= x_max,
        intersect=lambda s, e: intersect_x(s, e, x_max),
    )
    poly = clip_edge(
        poly,
        inside=lambda p: p[1] >= y_min,
        intersect=lambda s, e: intersect_y(s, e, y_min),
    )
    poly = clip_edge(
        poly,
        inside=lambda p: p[1] <= y_max,
        intersect=lambda s, e: intersect_y(s, e, y_max),
    )
    return poly


def _now_time() -> time:
    return datetime.now().time()


def _is_in_quiet_window(now_t: time, start: Optional[time], end: Optional[time]) -> bool:
    """判斷目前時間是否落在靜音時段，支援跨午夜。"""

    if start is None or end is None:
        return False
    if start <= end:
        return start <= now_t <= end
    return now_t >= start or now_t <= end


@dataclass(frozen=True)
class BedRoiZone:
    ip_address: str
    points: List[Tuple[float, float]]
    enabled: bool
    quiet_start: Optional[time]
    quiet_end: Optional[time]


class BedRoiLoader:
    """最小化的 ROI 讀取器，針對單一資料表查詢並排序座標。"""

    def __init__(self, db_config: DatabaseConfig) -> None:
        self._db_config = db_config
        self._lock = threading.Lock()
        self._conn = None

    def _ensure_conn(self):
        if psycopg is None:
            raise RuntimeError("psycopg is required for BedRoiLoader")
        with self._lock:
            if self._conn is None or self._conn.closed:
                self._conn = psycopg.connect(self._db_config.conninfo, autocommit=True)
            return self._conn

    def fetch_zone(self, ip_address: str) -> Optional[BedRoiZone]:
        sql = (
            "SELECT host(ip_address), points, enabled, quiet_start, quiet_end "
            "FROM ipc_bed_roi_zones "
            "WHERE ip_address = %s AND enabled = TRUE"
        )
        try:
            conn = self._ensure_conn()
            with conn.cursor() as cur:
                cur.execute(sql, (ip_address,))
                row = cur.fetchone()
        except Exception:
            return None
        if not row:
            return None

        _, points_json, enabled, quiet_start, quiet_end = row
        parsed_points: List[Tuple[float, float]] = []
        for point in points_json or []:
            try:
                x = float(point.get("x", 0.0))
                y = float(point.get("y", 0.0))
                parsed_points.append((x, y))
            except Exception:
                continue

        ordered = sort_polygon_points(parsed_points)
        return BedRoiZone(
            ip_address=ip_address,
            points=ordered,
            enabled=bool(enabled),
            quiet_start=quiet_start,
            quiet_end=quiet_end,
        )


@dataclass(frozen=True)
class BedExitResult:
    triggered: bool
    outside_ratio: float
    intersection_ratio: float
    log_line: str


class BedExitDetector:
    """檢查人物框是否離開床位 ROI。"""

    def __init__(
        self,
        ip_address: str,
        roi_loader: Callable[[str], Optional[BedRoiZone]],
        refresh_seconds: float = 1.0,
    ) -> None:
        self._ip_address = ip_address
        self._roi_loader = roi_loader
        self._refresh_seconds = refresh_seconds
        self._last_fetch_ts = 0.0
        self._zone: Optional[BedRoiZone] = None
        self._last_zone_signature: Optional[tuple] = None

    def update_ip(self, ip_address: str) -> None:
        if ip_address != self._ip_address:
            self._ip_address = ip_address
            self._zone = None
            self._last_fetch_ts = 0.0

    def _ensure_zone(self) -> Optional[BedRoiZone]:
        now_ts = time_module.time()
        if (
            self._zone is None
            or self._refresh_seconds <= 0.0
            or (now_ts - self._last_fetch_ts) >= self._refresh_seconds
        ):
            new_zone = self._roi_loader(self._ip_address)
            self._zone = new_zone  # 覆蓋舊設定（含 None）
            self._last_fetch_ts = now_ts
            sig = None
            if new_zone:
                sig = (
                    new_zone.enabled,
                    tuple(new_zone.points),
                    new_zone.quiet_start,
                    new_zone.quiet_end,
                )
            if sig != self._last_zone_signature:
                self._last_zone_signature = sig
                LOGGER.info(
                    "Bed ROI policy refreshed for %s: enabled=%s points=%s quiet=(%s, %s)",
                    self._ip_address,
                    getattr(new_zone, "enabled", None),
                    getattr(new_zone, "points", None),
                    getattr(new_zone, "quiet_start", None),
                    getattr(new_zone, "quiet_end", None),
                )
        return self._zone

    def evaluate(
        self,
        bbox_xyxy: Tuple[float, float, float, float],
        frame_shape: Tuple[int, int] | Tuple[int, int, int],
        now_time: Optional[time] = None,
    ) -> Optional[BedExitResult]:
        """回傳離床比例與觸發狀態；若沒有啟用設定則回傳 None。"""

        zone = self._ensure_zone()
        if zone is None or not zone.enabled or not zone.points:
            return None

        quiet_now = now_time or _now_time()
        if not _is_in_quiet_window(quiet_now, zone.quiet_start, zone.quiet_end):
            return None

        if len(frame_shape) >= 2:
            frame_h, frame_w = frame_shape[0], frame_shape[1]
        else:
            return None
        if frame_w <= 0 or frame_h <= 0:
            return None

        x1, y1, x2, y2 = bbox_xyxy
        if x2 <= x1 or y2 <= y1:
            return None

        rect_x1 = max(0.0, min(1.0, x1 / frame_w))
        rect_y1 = max(0.0, min(1.0, y1 / frame_h))
        rect_x2 = max(0.0, min(1.0, x2 / frame_w))
        rect_y2 = max(0.0, min(1.0, y2 / frame_h))

        intersection_poly = _clip_polygon_with_rect(
            polygon=zone.points,
            x_min=rect_x1,
            y_min=rect_y1,
            x_max=rect_x2,
            y_max=rect_y2,
        )
        intersection_area = _polygon_area(intersection_poly)
        rect_area = (rect_x2 - rect_x1) * (rect_y2 - rect_y1)
        if rect_area <= 0:
            return None

        intersection_ratio = 0.0 if rect_area == 0 else intersection_area / rect_area
        outside_ratio = 1.0 - intersection_ratio
        triggered = outside_ratio >= 0.75

        log_line = (
            f"bed_exit ip={self._ip_address} outside={outside_ratio:.3f} "
            f"inside={intersection_ratio:.3f} rect_area={rect_area:.4f} "
            f"points={len(zone.points)}"
        )

        return BedExitResult(
            triggered=triggered,
            outside_ratio=outside_ratio,
            intersection_ratio=intersection_ratio,
            log_line=log_line,
        )
