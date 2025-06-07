#!/usr/bin/env bash

set -e

OS="$(uname -s)"

SUDO=""
if [ "$(id -u)" -ne 0 ]; then
  SUDO="sudo"
fi

install_packages() {
  for pkg in "$@"; do
    $SUDO $INSTALL_CMD "$pkg"
  done
}

case "$OS" in
  Darwin)
    if ! command -v brew >/dev/null 2>&1; then
      echo "Homebrew not found. Please install Homebrew first: https://brew.sh/"
      exit 1
    fi
    INSTALL_CMD="brew list"
    packages=(cmake boost openssl nlohmann-json libpqxx libsodium pkg-config gflags glog libevent fmt double-conversion)
    for pkg in "${packages[@]}"; do
      brew list "$pkg" >/dev/null 2>&1 || brew install "$pkg"
    done
    ;;
  Linux)
    if command -v apt-get >/dev/null 2>&1; then
      $SUDO apt-get update
      INSTALL_CMD="apt-get install -y"
      packages=(build-essential cmake libboost-all-dev libssl-dev nlohmann-json3-dev libpqxx-dev libsodium-dev pkg-config git libdouble-conversion-dev libgoogle-glog-dev libgflags-dev libevent-dev libfmt-dev libunwind-dev libbz2-dev liblz4-dev liblzma-dev libzstd-dev libsnappy-dev libiberty-dev libdwarf-dev libaio-dev liburing-dev)
      for pkg in "${packages[@]}"; do
        dpkg -s "$pkg" >/dev/null 2>&1 || $SUDO apt-get install -y "$pkg"
      done
    else
      echo "Unsupported Linux distribution. Install dependencies manually."
      exit 1
    fi
    ;;
  *)
    echo "Unsupported OS: $OS"
    exit 1
    ;;
esac

# Update submodules
if [ -f .gitmodules ]; then
  git submodule update --init --recursive
fi

mkdir -p build
cd build
cmake ..

# Determine number of available CPU cores for parallel builds
if command -v nproc >/dev/null 2>&1; then
  NUM_CORES=$(nproc)
elif command -v sysctl >/dev/null 2>&1; then
  NUM_CORES=$(sysctl -n hw.ncpu)
else
  NUM_CORES=1
fi

# Build using all available cores
cmake --build . --parallel "$NUM_CORES"

