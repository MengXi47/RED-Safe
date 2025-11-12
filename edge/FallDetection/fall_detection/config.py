"""Application configuration helpers."""

from __future__ import annotations

import os
from dataclasses import dataclass


@dataclass(frozen=True)
class DatabaseConfig:
    host: str
    port: int
    name: str
    user: str
    password: str
    connect_timeout: int

    @property
    def conninfo(self) -> str:
        return (
            f"host={self.host} port={self.port} dbname={self.name} "
            f"user={self.user} password={self.password} connect_timeout={self.connect_timeout}"
        )


@dataclass(frozen=True)
class AppConfig:
    model_path: str
    model_conf: float
    model_imgsz: int
    show_windows: bool
    max_fps: float
    edge_id: str
    poll_interval: float
    stream_connect_timeout: float
    stream_reconnect_delay: float
    debug: bool
    mosaic_width: int
    mosaic_height: int
    mosaic_fps: float
    webrtc_host: str
    webrtc_port: int
    db: DatabaseConfig

    @classmethod
    def from_env(cls) -> "AppConfig":
        model_path = os.getenv("MODEL_PATH", "../model/yolo11n-pose.pt")
        model_conf = float(os.getenv("MODEL_CONF", "0.2"))
        model_imgsz = int(os.getenv("MODEL_IMGSZ", "640"))
        show_windows = os.getenv("MODEL_SHOW", "0").lower() in {
            "1",
            "true",
            "yes",
        }
        max_fps = float(os.getenv("MAX_FPS", "3"))
        edge_id = os.getenv("EDGE_ID", "RED-AAAAAAAA")
        poll_interval = float(os.getenv("IPC_POLL_INTERVAL", "1.0"))
        stream_connect_timeout = float(
            os.getenv("STREAM_CONNECT_TIMEOUT", "5.0")
        )
        stream_reconnect_delay = float(
            os.getenv("STREAM_RECONNECT_DELAY", "5.0")
        )
        debug = os.getenv("DEBUG", "0").lower() in {"1", "true", "yes"}
        mosaic_width = int(os.getenv("MOSAIC_WIDTH", "1920"))
        mosaic_height = int(os.getenv("MOSAIC_HEIGHT", "1080"))
        mosaic_fps = float(os.getenv("MOSAIC_FPS", "5.0"))
        webrtc_host = os.getenv("WEBRTC_HOST", "0.0.0.0")
        webrtc_port = int(os.getenv("WEBRTC_PORT", "8765"))

        db = DatabaseConfig(
            host=os.getenv("PGHOST", "localhost"),
            port=int(os.getenv("PGPORT", "5432")),
            name=os.getenv("PGDATABASE", "redsafedb"),
            user=os.getenv("PGUSER", "redsafedb"),
            password=os.getenv("PGPASSWORD", "redsafedb"),
            connect_timeout=int(os.getenv("PGCONNECT_TIMEOUT", "5")),
        )

        return cls(
            model_path=model_path,
            model_conf=model_conf,
            model_imgsz=model_imgsz,
            show_windows=show_windows,
            max_fps=max_fps,
            edge_id=edge_id,
            poll_interval=poll_interval,
            stream_connect_timeout=stream_connect_timeout,
            stream_reconnect_delay=stream_reconnect_delay,
            debug=debug,
            mosaic_width=mosaic_width,
            mosaic_height=mosaic_height,
            mosaic_fps=mosaic_fps,
            webrtc_host=webrtc_host,
            webrtc_port=webrtc_port,
            db=db,
        )
