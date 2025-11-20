#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG="edge-computer-vision:latest"
PLATFORM="linux/arm64"
CONTAINER_NAME="edge-computer-vision"

if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER_NAME"; then
  echo "[info] Stopping existing container $CONTAINER_NAME"
  docker rm -f "$CONTAINER_NAME"
fi

echo "[info] Building image $IMAGE_TAG for platform $PLATFORM"
docker buildx build --platform "$PLATFORM" --progress=plain -t "$IMAGE_TAG" .

echo "[info] Saveing container $CONTAINER_NAME"
docker save $IMAGE_TAG | gzip > edge-computer-vision-arm64.tar.gz
