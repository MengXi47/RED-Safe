FROM python:3.11-slim AS runtime

ENV PYTHONDONTWRITEBYTECODE=1 \
    PYTHONUNBUFFERED=1

WORKDIR /app

# Install runtime libs needed by OpenCV/PyTorch wheels.
RUN apt-get update && \
    apt-get install --no-install-recommends -y \
    libopenblas0 \
    libgl1 \
    libglib2.0-0 && \
    rm -rf /var/lib/apt/lists/*

# Install Python dependencies; --no-cache-dir keeps the layer smaller.
RUN pip install --no-cache-dir --upgrade pip && \
    pip install --no-cache-dir \
    torch --index-url https://download.pytorch.org/whl/cpu && \
    pip install --no-cache-dir \
    ultralytics \
    "lap>=0.5.12" \
    numpy \
    opencv-python \
    requests \
    psycopg[binary]

COPY fall_detection ./fall_detection
COPY main.py ./
COPY yolo11n-pose.pt ./model/yolo11n-pose.pt

# Default environment values; override as needed at runtime.
ENV MODEL_PATH=./model/yolo11n-pose.pt \
    PGHOST=127.0.0.1 \
    PGPORT=5432 \
    PGDATABASE=redsafedb \
    PGUSER=redsafedb \
    PGPASSWORD=redsafedb \
    STREAM_CONNECT_TIMEOUT=5.0 \
    STREAM_RECONNECT_DELAY=5.0

ENTRYPOINT ["python", "main.py"]
