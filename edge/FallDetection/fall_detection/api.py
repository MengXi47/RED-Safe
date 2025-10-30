"""HTTP API helpers."""

from __future__ import annotations

import atexit
import logging
from concurrent.futures import ThreadPoolExecutor
from datetime import datetime, timezone
from typing import Dict, List

import requests

from .config import AppConfig
from .db import IPCStreamConfig

LOGGER = logging.getLogger(__name__)

_executor = ThreadPoolExecutor(max_workers=4)
atexit.register(_executor.shutdown, wait=False)


def send_windows_to_api(
    config: AppConfig,
    stream: IPCStreamConfig,
    windows: List[Dict[str, float]],
) -> None:
    """Submit window payloads via background POST."""
    now_iso = (
        datetime.now(timezone.utc)
        .isoformat(timespec="milliseconds")
        .replace("+00:00", "Z")
    )
    payload: Dict[str, object] = {
        "windows": [
            {
                "window1": [windows[0]],
                "window2": [windows[1]],
                "window3": [windows[2]],
            }
        ],
        "edge_id": config.edge_id,
        "time": now_iso,
        "fall_sensitivity": stream.fall_sensitivity,
        "ip_address": stream.ip_address,
        "ipc_name": stream.custom_name or stream.ipc_name,
    }

    def _post(data: Dict[str, object]) -> None:
        try:
            response = requests.post(
                config.api_endpoint, json=data, timeout=config.request_timeout
            )
            response.raise_for_status()
            LOGGER.info(
                "Posted %s window batch to API for %s (%s) status=%s",
                len(windows),
                stream.ip_address,
                stream.custom_name or stream.ipc_name,
                response.status_code,
            )
        except requests.RequestException as exc:
            LOGGER.error(
                "Failed to deliver window batch for %s: %s", stream.ip_address, exc
            )

    _executor.submit(_post, payload)

