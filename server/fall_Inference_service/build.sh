#!/usr/bin/env bash
set -euo pipefail

IMAGE_TAG="fall-inference-service:latest"
PLATFORM="linux/amd64"
CONTAINER_NAME="fall-inference-service"

if docker ps -a --format '{{.Names}}' | grep -qx "$CONTAINER_NAME"; then
  echo "[info] Stopping existing container $CONTAINER_NAME"
  docker rm -f "$CONTAINER_NAME"
fi

echo "[info] Building image $IMAGE_TAG for platform $PLATFORM"
docker buildx build --platform "$PLATFORM" --progress=plain -t "$IMAGE_TAG" .

echo "[info] Saveing container $CONTAINER_NAME"
docker save $IMAGE_TAG | gzip > fall-inference-service-amd64.tar.gz
