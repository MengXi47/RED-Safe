"""Application entry orchestration."""

from __future__ import annotations

import logging
import threading
import time
from typing import Dict, Iterable

from .config import AppConfig
from .db import DatabaseClient, IPCStreamConfig
from .frame_bus import FrameHub
from .mosaic import MosaicComposer, MosaicSettings
from .stream import StreamWorker
from .webrtc import WebRTCServer

LOGGER = logging.getLogger(__name__)


class StreamCoordinator:
    """Manage stream workers to mirror DB rows."""

    def __init__(
        self,
        app_config: AppConfig,
        frame_hub: FrameHub,
        db_client: DatabaseClient,
    ) -> None:
        self.app_config = app_config
        self._workers: Dict[str, StreamWorker] = {}
        self._lock = threading.Lock()
        self._frame_hub = frame_hub
        self._db_client = db_client

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
                    worker = StreamWorker(
                        self.app_config,
                        stream,
                        self._frame_hub,
                        self._db_client,
                    )
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
        level=logging.DEBUG if app_config.debug else logging.INFO,
        format="%(asctime)s %(levelname)s [%(name)s] %(message)s",
    )
    LOGGER.info("Starting fall detection service with EDGE_ID=%s", app_config.edge_id)

    frame_hub = FrameHub()
    mosaic = MosaicComposer(
        frame_hub,
        MosaicSettings(
            width=app_config.mosaic_width,
            height=app_config.mosaic_height,
            fps=app_config.mosaic_fps,
            max_tiles=4,
        ),
    )
    mosaic.start()

    webrtc_server = WebRTCServer(
        composer=mosaic,
        host=app_config.webrtc_host,
        port=app_config.webrtc_port,
    )
    webrtc_server.start()

    db_client = DatabaseClient(app_config.db)
    coordinator = StreamCoordinator(app_config, frame_hub, db_client)

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
        webrtc_server.stop()
        mosaic.stop()
        LOGGER.info("Shutdown complete.")
