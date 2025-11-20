"""Database access helpers for connected IPC metadata."""

from __future__ import annotations

import contextlib
import logging
import threading
from dataclasses import dataclass
from datetime import datetime, timezone
from typing import Iterable, List, Tuple

import psycopg
from psycopg.types.json import Json

from .bednight import BedRoiZone, sort_polygon_points
from .inactivity import InactivityPolicy
from .config import DatabaseConfig

LOGGER = logging.getLogger(__name__)


@dataclass(frozen=True)
class IPCStreamConfig:
    ip_address: str
    mac_address: str
    ipc_name: str | None
    custom_name: str | None
    ipc_account: str | None
    ipc_password: str | None
    fall_detection_enabled: bool

    @property
    def stream_url(self) -> str:
        account = self.ipc_account or ""
        password = self.ipc_password or ""
        auth = f"{account}:{password}@"
        return f"rtsp://{auth}{self.ip_address}:554/stream1"


class DatabaseClient:
    """Minimal psycopg wrapper with auto-reconnect for polling use."""

    def __init__(self, config: DatabaseConfig) -> None:
        self._config = config
        self._lock = threading.Lock()
        self._conn: psycopg.Connection | None = None

    def _connect(self) -> psycopg.Connection:
        LOGGER.info("Connecting to PostgreSQL %s:%s", self._config.host, self._config.port)
        return psycopg.connect(self._config.conninfo, autocommit=True)

    def _ensure_connection(self) -> psycopg.Connection:
        with self._lock:
            if self._conn is None or self._conn.closed:
                self._conn = self._connect()
            return self._conn

    def fetch_connected_ipc(self) -> List[IPCStreamConfig]:
        sql = (
            "SELECT "
            "host(ci.ip_address), "
            "ci.mac_address, "
            "ci.ipc_name, "
            "ci.custom_name, "
            "ci.ipc_account, "
            "ci.ipc_password, "
            "COALESCE(fdp.enabled, FALSE) as fall_enabled "
            "FROM connected_ipc ci "
            "LEFT JOIN ipc_fall_detection_policy fdp ON fdp.ip_address = ci.ip_address"
        )
        try:
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql)
                rows = cur.fetchall()
        except (psycopg.OperationalError, psycopg.InterfaceError) as exc:
            LOGGER.warning("Database query failed: %s. Reconnecting.", exc)
            with self._lock:
                if self._conn is not None:
                    with contextlib.suppress(Exception):
                        self._conn.close()
                    self._conn = None
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql)
                rows = cur.fetchall()

        return [
            IPCStreamConfig(
                ip_address=row[0],
                mac_address=row[1],
                ipc_name=row[2],
                custom_name=row[3],
                ipc_account=row[4],
                ipc_password=row[5],
                fall_detection_enabled=bool(row[6]),
            )
            for row in rows
        ]

    def log_edge_event(
        self,
        event_type: str,
        payload: dict | None = None,
        occurred_at: datetime | None = None,
    ) -> None:
        """將事件寫入 edge_events 表（事件時間預設 UTC now）。"""

        ts = occurred_at or datetime.now(timezone.utc)
        sql = (
            "INSERT INTO edge_events (event_type, payload, occurred_at) "
            "VALUES (%s, %s, %s)"
        )
        args = (event_type, Json(payload) if payload is not None else None, ts)

        def _execute() -> None:
            conn = self._ensure_connection()
            with self._lock:
                with conn.cursor() as cur:
                    cur.execute(sql, args)

        try:
            _execute()
        except (psycopg.OperationalError, psycopg.InterfaceError) as exc:
            LOGGER.warning("Database insert failed: %s. Reconnecting.", exc)
            with self._lock:
                if self._conn is not None:
                    with contextlib.suppress(Exception):
                        self._conn.close()
                    self._conn = None
            _execute()
        except Exception as exc:
            LOGGER.warning("log_edge_event error (%s): %s", event_type, exc)

    def fetch_bed_roi_zones(self) -> List[BedRoiZone]:
        """取得各 IPC 的床位 ROI 設定，並將點排序。"""

        sql = (
            "SELECT host(ip_address), points, enabled, quiet_start, quiet_end "
            "FROM ipc_bed_roi_zones "
            "WHERE enabled = TRUE"
        )
        try:
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql)
                rows = cur.fetchall()
        except (psycopg.OperationalError, psycopg.InterfaceError) as exc:
            LOGGER.warning("Database query failed: %s. Reconnecting.", exc)
            with self._lock:
                if self._conn is not None:
                    with contextlib.suppress(Exception):
                        self._conn.close()
                    self._conn = None
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql)
                rows = cur.fetchall()

        zones: List[BedRoiZone] = []
        for row in rows:
            ip_addr, points_json, enabled, quiet_start, quiet_end = row
            parsed_points: List[Tuple[float, float]] = []
            for point in points_json or []:
                try:
                    x = float(point.get("x", 0.0))
                    y = float(point.get("y", 0.0))
                    parsed_points.append((x, y))
                except Exception:
                    LOGGER.debug("Skip invalid ROI point %s for %s", point, ip_addr)
            ordered_points = sort_polygon_points(parsed_points)
            zones.append(
                BedRoiZone(
                    ip_address=ip_addr,
                    points=ordered_points,
                    enabled=bool(enabled),
                    quiet_start=quiet_start,
                    quiet_end=quiet_end,
                )
            )
        return zones

    def fetch_bed_roi_zone(self, ip_address: str) -> BedRoiZone | None:
        """取得單一 IPC 的床位 ROI 設定，並將點排序。"""

        sql = (
            "SELECT host(ip_address), points, enabled, quiet_start, quiet_end "
            "FROM ipc_bed_roi_zones "
            "WHERE enabled = TRUE AND ip_address = %s"
        )
        try:
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql, (ip_address,))
                row = cur.fetchone()
        except (psycopg.OperationalError, psycopg.InterfaceError) as exc:
            LOGGER.warning("Database query failed: %s. Reconnecting.", exc)
            with self._lock:
                if self._conn is not None:
                    with contextlib.suppress(Exception):
                        self._conn.close()
                    self._conn = None
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql, (ip_address,))
                row = cur.fetchone()

        if not row:
            return None

        ip_addr, points_json, enabled, quiet_start, quiet_end = row
        parsed_points: List[Tuple[float, float]] = []
        for point in points_json or []:
            try:
                x = float(point.get("x", 0.0))
                y = float(point.get("y", 0.0))
                parsed_points.append((x, y))
            except Exception:
                LOGGER.debug("Skip invalid ROI point %s for %s", point, ip_addr)
        ordered_points = sort_polygon_points(parsed_points)
        return BedRoiZone(
            ip_address=ip_addr,
            points=ordered_points,
            enabled=bool(enabled),
            quiet_start=quiet_start,
            quiet_end=quiet_end,
        )

    def fetch_inactivity_policy(self, ip_address: str) -> InactivityPolicy | None:
        """取得 IPC 的靜止偵測策略。"""

        sql = (
            "SELECT enabled, idle_minutes, quiet_start, quiet_end, quiet_enabled "
            "FROM ipc_inactivity_policy WHERE ip_address = %s"
        )
        try:
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql, (ip_address,))
                row = cur.fetchone()
        except (psycopg.OperationalError, psycopg.InterfaceError) as exc:
            LOGGER.warning("Database query failed: %s. Reconnecting.", exc)
            with self._lock:
                if self._conn is not None:
                    with contextlib.suppress(Exception):
                        self._conn.close()
                    self._conn = None
            conn = self._ensure_connection()
            with conn.cursor() as cur:
                cur.execute(sql, (ip_address,))
                row = cur.fetchone()

        if not row:
            return None

        enabled, idle_minutes, quiet_start, quiet_end, quiet_enabled = row
        return InactivityPolicy(
            enabled=bool(enabled),
            idle_minutes=int(idle_minutes),
            quiet_start=quiet_start,
            quiet_end=quiet_end,
            quiet_enabled=bool(quiet_enabled),
        )

    def close(self) -> None:
        with self._lock:
            if self._conn is not None:
                with contextlib.suppress(Exception):
                    self._conn.close()
                self._conn = None
