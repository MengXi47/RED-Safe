#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG="cam:latest"
CONTAINER_NAME="cam_1"
PLATFORM="linux/arm64"

if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER_NAME"; then
  echo "[info] Stopping existing container $CONTAINER_NAME"
  docker rm -f "$CONTAINER_NAME"
fi

echo "[info] Building image $IMAGE_TAG for platform $PLATFORM"
docker buildx build --platform "$PLATFORM" --progress=plain -t "$IMAGE_TAG" .

echo "[info] Starting container $CONTAINER_NAME"
docker run -d --name "$CONTAINER_NAME" --network host "$IMAGE_TAG"
