"""Fall detection service entrypoint."""

from __future__ import annotations

from Vision.app import run
from Vision.config import AppConfig


def main() -> None:
    config = AppConfig.from_env()
    run(config)


if __name__ == "__main__":
    main()

