"""Database access helpers for connected IPC metadata."""

from __future__ import annotations

import contextlib
import logging
import threading
from dataclasses import dataclass
from typing import Iterable, List

import psycopg

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
    fall_sensitivity: int | None

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
            "ip_address::text, "
            "mac_address, "
            "ipc_name, "
            "custom_name, "
            "ipc_account, "
            "ipc_password, "
            "fall_sensitivity "
            "FROM connected_ipc"
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
                fall_sensitivity=int(row[6]) if row[6] is not None else None,
            )
            for row in rows
        ]

    def close(self) -> None:
        with self._lock:
            if self._conn is not None:
                with contextlib.suppress(Exception):
                    self._conn.close()
                self._conn = None
