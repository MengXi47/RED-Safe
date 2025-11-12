"""負責將跌倒事件上報到雲端 API。"""

from __future__ import annotations

import base64
import logging
from typing import Optional

import requests

from .config import AppConfig
from .db import IPCStreamConfig

LOGGER = logging.getLogger(__name__)


class FallEventReporter:
    """封裝 HTTP 請求流程，避免在偵測執行緒阻塞過久。"""

    def __init__(self, app_config: AppConfig) -> None:
        self._config = app_config
        self._session = requests.Session()

    def report(self, stream: IPCStreamConfig, snapshot: Optional[bytes], event_time_iso: str) -> None:
        if not self._config.fall_event_enabled:
            return

        endpoint = (self._config.fall_event_endpoint or "").strip()
        if not endpoint:
            LOGGER.debug("fall_event_endpoint 未設定，略過上報")
            return

        payload = {
            "edge_id": self._config.edge_id,
            "ip_address": stream.ip_address,
            "ipc_name": stream.custom_name or stream.ipc_name or stream.ip_address,
            "location": self._config.fall_event_location
            or stream.custom_name
            or stream.ipc_name
            or stream.ip_address,
            "event_time": event_time_iso,
        }

        if snapshot:
            payload["snapshot_base64"] = base64.b64encode(snapshot).decode("ascii")
            payload["snapshot_mime_type"] = "image/jpeg"

        try:
            response = self._session.post(
                endpoint,
                json=payload,
                timeout=self._config.fall_event_timeout,
            )
            if response.status_code >= 400:
                LOGGER.error(
                    "Fall event上報失敗 status=%s body=%s",
                    response.status_code,
                    response.text[:200],
                )
        except requests.RequestException as exc:  # pragma: no cover - 網路例外預期外
            LOGGER.error(
                "呼叫 %s 失敗 (%s)",
                endpoint,
                exc,
            )
