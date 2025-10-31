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
    libdouble-conversion-dev \
    libgflags-dev \
    libgoogle-glog-dev \
    libevent-dev \
    libfmt-dev \
    libssl-dev \
    libsodium-dev \
    liblz4-dev \
    libzstd-dev \
    libsnappy-dev \
    libunwind-dev \
    libjemalloc-dev \
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

# 引入最新版 FastFloat
RUN git clone --depth=1 https://github.com/fastfloat/fast_float.git /tmp/fast_float \
    && cmake -S /tmp/fast_float -B /tmp/fast_float/build -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/fast_float \
    -DBUILD_SHARED_LIBS=ON \
    && cmake --build /tmp/fast_float/build -j \
    && cmake --install /tmp/fast_float/build \
    && rm -rf /tmp/fast_float

ENV CMAKE_PREFIX_PATH=/opt/fast_float:$CMAKE_PREFIX_PATH \
    LD_LIBRARY_PATH=/opt/fast_float/lib:$LD_LIBRARY_PATH

# 建置最新 Folly 供後續 CMake 尋找
RUN git clone --depth=1 https://github.com/facebook/folly.git /tmp/folly \
    && cmake -S /tmp/folly -B /tmp/folly/build -GNinja \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/opt/folly \
    -DBUILD_SHARED_LIBS=ON \
    -DFOLLY_USE_SYMBOLIZER=OFF \
    -DFOLLY_HAVE_LIBJEMALLOC=ON \
    && cmake --build /tmp/folly/build -j \
    && cmake --install /tmp/folly/build \
    && rm -rf /tmp/folly

ENV CMAKE_PREFIX_PATH=/opt/folly:$CMAKE_PREFIX_PATH \
    LD_LIBRARY_PATH=/opt/folly/lib:$LD_LIBRARY_PATH

# https://github.com/boostorg/mqtt5
RUN set -eux; \
    git clone --depth=1 https://github.com/boostorg/mqtt5 /tmp/mqtt5; \
    mkdir -p /usr/local/include/boost; \
    cp -r /tmp/mqtt5/include/boost/mqtt5 /usr/local/include/boost/; \
    rm -rf /tmp/mqtt5

COPY . .

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

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    libprotobuf-dev \
    libgrpc++-dev \
    libboost-system1.83.0 \
    libboost-thread1.83.0 \
    libboost-context1.83.0 \
    libboost-filesystem1.83.0 \
    libboost-program-options1.83.0 \
    libdouble-conversion3 \
    libgflags2.2 \
    libgoogle-glog0v6t64 \
    libevent-2.1-7 \
    libfmt10 \
    libssl3 \
    libsodium23 \
    liblz4-1 \
    libzstd1 \
    libsnappy1v5 \
    libunwind8 \
    libjemalloc2 \
    libcurl4 \
    libpq-dev \
    libpqxx-dev \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/fast_float /opt/fast_float
COPY --from=build /opt/folly /opt/folly
ENV LD_LIBRARY_PATH=/opt/folly/lib:/opt/fast_float/lib:$LD_LIBRARY_PATH
ENV CMAKE_PREFIX_PATH=/opt/folly:/opt/fast_float:$CMAKE_PREFIX_PATH

ENV RED_SAFE_EDGE_ID="RED-AAAAAAAA"
ENV RED_SAFE_EDGE_VERSION="0.0.1"
ENV RED_SAFE_NETWORK_INTERFACE="eth0"
ENV RED_SAFE_IPTOOL_TARGET="localhost:20002"
ENV RED_SAFE_SERVER_URL="https://api.redsafe-tw.com"
ENV RED_SAFE_MQTT_BROKER="mqtt.redsafe-tw.com/mqtt"
ENV RED_SAFE_MQTT_PORT="443"
ENV RED_SAFE_MQTT_USERNAME="redsafemqtt"
ENV RED_SAFE_MQTT_PASSWORD="redsafemqtt"
ENV RED_SAFE_GRPC_PORT="20001"
ENV RED_SAFE_HEARTBEAT_MS=1000
ENV RED_SAFE_IPCSCAN_TIMEOUT_MS=3000

COPY --from=build /opt/RED_SAFE_EDGE_CORE /opt/RED_SAFE_EDGE_CORE

EXPOSE 20001
WORKDIR /opt/RED_SAFE_EDGE_CORE/bin
ENTRYPOINT ["/opt/RED_SAFE_EDGE_CORE/bin/RED_SAFE_EDGE_CORE"]
