# Repository Guidelines

## Project Structure & Module Organization
- `django_web/`: marketing + dashboard prototype; logic in `main/`, templates in `templates/`, static files in `static/`.
- `server/<service>_service/`: Spring Boot backends (edge, heartbeat, ios, user) with shared config in `server/config`.
- `edge/RED_SAFE_EDGE_CORE/`: C++ inference engine; protobufs in `proto/`, helper scripts in root `cmake/`.
- `fall_detection_mdp/`: MediaPipe + PyTorch training pipeline, checkpoints in `model/`; `fall_model/` hosts the YOLO demo.
- `raspberry_web/raspberry_contral/`: Django control UI for Raspberry Pi; `iOSapp/RED Safe/` Swift client workspace.
- `web/` static site bundle and `docs/` architecture references for planning.

## Build, Test, and Development Commands
- Django apps: `cd <app> && python manage.py migrate && python manage.py runserver`, then `python manage.py test` for regression checks.
- Spring services: `cd server/<service>_service && ./mvnw spring-boot:run`; verify with `./mvnw test` and package with `./mvnw package`.
- Edge core: `cmake -S edge/RED_SAFE_EDGE_CORE -B build/edge && cmake --build build/edge`; once tests exist run `ctest --test-dir build/edge`.
- ML stack: create a venv, `pip install -r fall_detection_mdp/requirements.txt`, run `python fall_detection_mdp/train.py` to train or `python fall_detection_mdp/main.py` for live inference; `python fall_model/main.py --video sample.mp4` demos YOLO.

## Coding Style & Naming Conventions
- Python follows PEP 8 with four-space indents, snake case, and CamelCase classes; add type hints when practical and run `black` plus `isort`.
- Java keeps standard Spring layout: packages `com.redsafe.<service>`, PascalCase classes, camelCase members, and clear controller/service/repository layers.
- C++ keeps braces on declaration lines, prefers RAII and STL helpers, and should pass `clang-format`.
- Swift sticks to Xcode defaults, struct-based views, and descriptive enum cases.

## Testing Guidelines
- Django modules keep tests beside apps (for example `main/tests/`) and run `python manage.py test` before pushing.
- Spring services mirror production packages under `src/test/java`; keep tests fast and mock external calls.
- Edge modules add GoogleTest suites under `edge/RED_SAFE_EDGE_CORE/src/tests/` and wire them into CMake so `ctest` stays useful.
- ML components create `pytest` suites in `fall_detection_mdp/tests/`, covering preprocessing, feature extraction, and model wrappers.

## Commit & Pull Request Guidelines
- Write terse, imperative commit subjects (e.g., `Harden heartbeat timeout`); bilingual bodies are fine but include an English summary line.
- Reference issues with `Refs #123` or `Fixes #123`, note generated assets or migrations, and avoid committing secrets or large binaries.
- Pull requests summarise intent, list affected modules, attach UI captures or command output, and state which tests ran.
- Tag domain reviewers touched (web, edge, ML, mobile) and confirm lint/test commands ran.
