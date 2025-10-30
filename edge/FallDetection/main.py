"""Fall detection service entrypoint."""

from __future__ import annotations

from fall_detection.app import run
from fall_detection.config import AppConfig


def main() -> None:
    config = AppConfig.from_env()
    run(config)


if __name__ == "__main__":
    main()

