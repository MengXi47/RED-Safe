########################
# Build stage
########################
FROM --platform=linux/amd64 debian:trixie-slim AS build
ARG DEBIAN_FRONTEND=noninteractive

# 基本編譯工具與相依套件
RUN apt-get update \
    && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    pkg-config \
    git \
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
    wget \
    unzip \
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

ARG LIBTORCH_URL="https://download.pytorch.org/libtorch/cpu/libtorch-cxx11-abi-shared-with-deps-2.3.1%2Bcpu.zip"
RUN wget -qO /tmp/libtorch.zip "${LIBTORCH_URL}" \
    && rm -rf /opt/libtorch \
    && unzip -q /tmp/libtorch.zip -d /opt \
    && rm /tmp/libtorch.zip

ENV TORCH_HOME=/opt/libtorch \
    CMAKE_PREFIX_PATH=/opt/libtorch/share/cmake:$CMAKE_PREFIX_PATH \
    LD_LIBRARY_PATH=/opt/libtorch/lib:$LD_LIBRARY_PATH

COPY . .

RUN rm -rf libtorch

# 建置與安裝到臨時目的地
RUN cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build -j \
    && mkdir -p /opt/fall_inference_service/bin \
    && cp build/bin/fall_inference_service_bin /opt/fall_inference_service/bin/ \
    && cp fall_probability_model_ts.pt /opt/fall_inference_service/bin/

########################
# Runtime stage
########################
FROM --platform=linux/amd64 debian:trixie-slim AS runtime
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
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/fast_float /opt/fast_float
COPY --from=build /opt/folly /opt/folly
COPY --from=build /opt/fall_inference_service /opt/fall_inference_service
COPY --from=build /opt/libtorch /opt/libtorch
ENV LD_LIBRARY_PATH=/opt/libtorch/lib:/opt/folly/lib:/opt/fast_float/lib:$LD_LIBRARY_PATH
ENV CMAKE_PREFIX_PATH=/opt/libtorch/share/cmake:/opt/folly:/opt/fast_float:$CMAKE_PREFIX_PATH
ENV TORCH_HOME=/opt/libtorch
COPY --from=build /opt/fall_inference_service /opt/fall_inference_service

EXPOSE 30050
WORKDIR /opt/fall_inference_service/bin
ENTRYPOINT ["/opt/fall_inference_service/bin/fall_inference_service_bin"]
