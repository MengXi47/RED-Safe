"""Application entry orchestration."""

from __future__ import annotations

import logging
import threading
import time
from typing import Dict, Iterable

from .config import AppConfig
from .db import DatabaseClient, IPCStreamConfig
from .stream import StreamWorker

LOGGER = logging.getLogger(__name__)


class StreamCoordinator:
    """Manage stream workers to mirror DB rows."""

    def __init__(self, app_config: AppConfig) -> None:
        self.app_config = app_config
        self._workers: Dict[str, StreamWorker] = {}
        self._lock = threading.Lock()

    def sync(self, streams: Iterable[IPCStreamConfig]) -> None:
        desired = {stream.stream_url: stream for stream in streams}
        with self._lock:
            # Add or update workers
            for url, stream in desired.items():
                worker = self._workers.get(url)
                if worker is None:
                    LOGGER.info(
                        "Discovered new stream %s (ip=%s, name=%s)",
                        url,
                        stream.ip_address,
                        stream.custom_name or stream.ipc_name,
                    )
                    worker = StreamWorker(self.app_config, stream)
                    self._workers[url] = worker
                    worker.start()
                else:
                    if worker.stream != stream:
                        LOGGER.info(
                            "Updating stream metadata %s (ip=%s, name=%s)",
                            url,
                            stream.ip_address,
                            stream.custom_name or stream.ipc_name,
                        )
                        worker.update_stream(stream)

            # Stop removed workers
            for url in list(self._workers.keys()):
                if url not in desired:
                    LOGGER.info("Removing stream %s", url)
                    worker = self._workers.pop(url)
                    worker.stop()

    def shutdown(self) -> None:
        with self._lock:
            for worker in list(self._workers.values()):
                worker.stop()
            self._workers.clear()


def run(app_config: AppConfig) -> None:
    logging.basicConfig(
        level=logging.INFO,
        format="%(asctime)s %(levelname)s [%(name)s] %(message)s",
    )
    LOGGER.info("Starting fall detection service with EDGE_ID=%s", app_config.edge_id)

    db_client = DatabaseClient(app_config.db)
    coordinator = StreamCoordinator(app_config)

    try:
        while True:
            try:
                streams = db_client.fetch_connected_ipc()
                coordinator.sync(streams)
            except Exception as exc:
                LOGGER.exception("Failed to refresh streams: %s", exc)
            time.sleep(app_config.poll_interval)
    except KeyboardInterrupt:
        LOGGER.info("Shutting down due to keyboard interrupt.")
    finally:
        coordinator.shutdown()
        db_client.close()
        LOGGER.info("Shutdown complete.")
