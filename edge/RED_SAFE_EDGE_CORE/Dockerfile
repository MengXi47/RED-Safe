# syntax=docker/dockerfile:1

########################
# Build stage
########################
FROM arm64v8/debian:trixie AS build
ARG DEBIAN_FRONTEND=noninteractive

# 基本編譯工具與相依套件
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential \
       cmake \
       ninja-build \
       pkg-config \
       git \
       curl \
       ca-certificates \
       protobuf-compiler \
       protobuf-compiler-grpc \
       libprotobuf-dev \
       libgrpc-dev \
       libgrpc++-dev \
       libboost-all-dev \
       nlohmann-json3-dev \
       libcurl4-openssl-dev \
       libpq-dev \
       libpqxx-dev \
       gcc-14 \
       g++-14 \
    && rm -rf /var/lib/apt/lists/*

# 使用 GCC 14
ENV CC=gcc-14 \
    CXX=g++-14

WORKDIR /workspace
COPY . .

# 方案A：補上 Boost.MQTT5（header-only）到系統 include path
# 來源：https://github.com/boostorg/mqtt5
RUN set -eux; \
    git clone --depth=1 https://github.com/boostorg/mqtt5 /tmp/mqtt5; \
    mkdir -p /usr/local/include/boost; \
    cp -r /tmp/mqtt5/include/boost/mqtt5 /usr/local/include/boost/; \
    rm -rf /tmp/mqtt5

# 建置與安裝到臨時目的地
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j \
    && mkdir -p /opt/RED_SAFE_EDGE_CORE/bin \
    && cp build/bin/RED_SAFE_EDGE_CORE /opt/RED_SAFE_EDGE_CORE/bin/

########################
# Runtime stage
########################
FROM arm64v8/debian:trixie AS runtime
ARG DEBIAN_FRONTEND=noninteractive

# 執行期相依（可再依 ldd 精簡）
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       libprotobuf-dev \
       libgrpc++-dev \
       libboost-system1.83.0 \
       libboost-thread1.83.0 \
       libcurl4 \
       libpq-dev \
       libpqxx-dev \
       ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/RED_SAFE_EDGE_CORE /opt/RED_SAFE_EDGE_CORE

EXPOSE 20001
WORKDIR /opt/RED_SAFE_EDGE_CORE/bin
ENTRYPOINT ["/opt/RED_SAFE_EDGE_CORE/bin/RED_SAFE_EDGE_CORE"]
