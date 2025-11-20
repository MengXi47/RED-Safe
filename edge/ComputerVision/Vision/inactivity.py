"""靜止偵測模組。"""

from __future__ import annotations

import logging
import math
from dataclasses import dataclass
from datetime import datetime, time as dt_time
from typing import Callable, NamedTuple, Optional

LOGGER = logging.getLogger(__name__)


@dataclass(frozen=True)
class InactivityPolicy:
    """單一 IPC 的靜止偵測策略。"""

    enabled: bool
    idle_minutes: int
    quiet_start: dt_time | None
    quiet_end: dt_time | None
    quiet_enabled: bool


class InactivityResult(NamedTuple):
    """靜止檢測輸出。"""

    triggered: bool
    idle_seconds: float
    log_line: str


class InactivityDetector:
    """根據 bounding box 移動量追蹤靜止狀態。"""

    def __init__(
        self,
        ip_address: str,
        policy_loader: Callable[[str], Optional[InactivityPolicy]],
        refresh_seconds: float = 5.0,
    ) -> None:
        self._ip_address = ip_address
        self._policy_loader = policy_loader
        self._refresh_seconds = refresh_seconds
        self._policy: InactivityPolicy | None = None
        self._next_refresh_ts = 0.0
        self._last_pos_by_pid: dict[int, tuple[float, float]] = {}
        self._last_active_ts_by_pid: dict[int, float] = {}
        self._movement_px_threshold = 8.0  # TODO: 調整靜止判定的移動容忍值

    def is_enabled(self) -> bool:
        """回傳策略是否啟用（不含免偵測時段判斷）。"""
        return bool(self._policy and self._policy.enabled)

    def update_ip(self, ip_address: str) -> None:
        """在 Stream 更新時同步 IP，避免沿用舊策略。"""
        self._ip_address = ip_address
        self._policy = None
        self._next_refresh_ts = 0.0
        self._last_pos_by_pid.clear()
        self._last_active_ts_by_pid.clear()

    def _refresh_policy_if_needed(self, now_s: float) -> None:
        if now_s < self._next_refresh_ts:
            return
        prev_policy = self._policy
        try:
            self._policy = self._policy_loader(self._ip_address)
        except Exception as exc:
            LOGGER.debug("Failed to refresh inactivity policy for %s: %s", self._ip_address, exc)
        self._next_refresh_ts = now_s + self._refresh_seconds
        if self._policy != prev_policy:
            LOGGER.info(
                "Inactivity policy refreshed for %s: enabled=%s idle_minutes=%s quiet=(%s, %s) quiet_enabled=%s",
                self._ip_address,
                getattr(self._policy, "enabled", None),
                getattr(self._policy, "idle_minutes", None),
                getattr(self._policy, "quiet_start", None),
                getattr(self._policy, "quiet_end", None),
                getattr(self._policy, "quiet_enabled", None),
            )

    @staticmethod
    def _is_quiet_now(policy: InactivityPolicy, now_dt: datetime) -> bool:
        """處理跨日的免偵測時段。"""
        if not policy.quiet_enabled or policy.quiet_start is None or policy.quiet_end is None:
            return False

        start = policy.quiet_start
        end = policy.quiet_end
        now_t = now_dt.time()

        if start == end:
            # 同時間表示全天免偵測
            return True

        if start < end:
            return start <= now_t < end
        # 跨日，例如 21:00 ~ 07:00
        return now_t >= start or now_t < end

    def evaluate(
        self,
        pid: int,
        bbox_xyxy: tuple[float, float, float, float],
        timestamp_ms: int,
    ) -> Optional[InactivityResult]:
        """根據當前 bbox 判斷是否靜止超過閾值。"""
        now_s = timestamp_ms / 1000.0
        self._refresh_policy_if_needed(now_s)
        policy = self._policy

        if policy is None or not policy.enabled:
            self._last_pos_by_pid.pop(pid, None)
            self._last_active_ts_by_pid.pop(pid, None)
            return None

        now_dt = datetime.fromtimestamp(now_s)
        if self._is_quiet_now(policy, now_dt):
            return None

        x1, y1, x2, y2 = bbox_xyxy
        cx = (x1 + x2) * 0.5
        cy = (y1 + y2) * 0.5

        last_pos = self._last_pos_by_pid.get(pid)
        moved = True

        if last_pos is not None:
            dx = cx - last_pos[0]
            dy = cy - last_pos[1]
            dist = math.hypot(dx, dy)
            moved = dist > self._movement_px_threshold

        if moved:
            self._last_pos_by_pid[pid] = (cx, cy)
            self._last_active_ts_by_pid[pid] = now_s
            return None

        last_active = self._last_active_ts_by_pid.get(pid)
        if last_active is None:
            self._last_active_ts_by_pid[pid] = now_s
            self._last_pos_by_pid[pid] = (cx, cy)
            return None

        idle_seconds = now_s - last_active
        idle_threshold = max(policy.idle_minutes, 0) * 60.0
        triggered = idle_seconds >= idle_threshold > 0

        log_line = (
            f"ID {pid} idle={round(idle_seconds,1)}s "
            f"threshold={idle_threshold}s "
            f"quiet={self._is_quiet_now(policy, now_dt)}"
        )
        return InactivityResult(triggered=triggered, idle_seconds=idle_seconds, log_line=log_line)
