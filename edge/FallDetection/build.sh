#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG="edge-fall-detection:latest"
PLATFORM="linux/arm64"
CONTAINER_NAME="edge-fall-detection"

if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER_NAME"; then
  echo "[info] Stopping existing container $CONTAINER_NAME"
  docker rm -f "$CONTAINER_NAME"
fi

echo "[info] Building image $IMAGE_TAG for platform $PLATFORM"
docker buildx build --platform "$PLATFORM" --progress=plain -t "$IMAGE_TAG" .

echo "[info] Saveing container $CONTAINER_NAME"
docker save $IMAGE_TAG | gzip > edge-fall-detection-arm64.tar.gz
