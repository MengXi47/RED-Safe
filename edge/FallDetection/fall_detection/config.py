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
    api_endpoint: str
    edge_id: str
    window_frames: int
    window_batch_size: int
    poll_interval: float
    request_timeout: float
    stream_connect_timeout: float
    stream_reconnect_delay: float
    db: DatabaseConfig

    @classmethod
    def from_env(cls) -> "AppConfig":
        model_path = os.getenv("MODEL_PATH", "../model/yolo11n-pose.pt")
        model_conf = float(os.getenv("MODEL_CONF", "0.2"))
        model_imgsz = int(os.getenv("MODEL_IMGSZ", "640"))
        show_windows = os.getenv("MODEL_SHOW", "0").lower() in {"1", "true", "yes"}
        max_fps = float(os.getenv("MAX_FPS", "3"))
        api_endpoint = os.getenv("FALL_WINDOW_API", "https://api.redsafe-tw.com/edge/test")
        edge_id = os.getenv("EDGE_ID", "RED-AAAAAAAA")
        window_frames = int(os.getenv("WINDOW_FRAMES", "3"))
        window_batch_size = int(os.getenv("WINDOW_BATCH_SIZE", "3"))
        poll_interval = float(os.getenv("IPC_POLL_INTERVAL", "1.0"))
        request_timeout = float(os.getenv("REQUEST_TIMEOUT", "5.0"))
        stream_connect_timeout = float(os.getenv("STREAM_CONNECT_TIMEOUT", "5.0"))
        stream_reconnect_delay = float(os.getenv("STREAM_RECONNECT_DELAY", "5.0"))

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
            api_endpoint=api_endpoint,
            edge_id=edge_id,
            window_frames=window_frames,
            window_batch_size=window_batch_size,
            poll_interval=poll_interval,
            request_timeout=request_timeout,
            stream_connect_timeout=stream_connect_timeout,
            stream_reconnect_delay=stream_reconnect_delay,
            db=db,
        )
