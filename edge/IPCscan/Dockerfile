# syntax=docker/dockerfile:1

FROM arm64v8/debian:trixie AS build
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       build-essential \
       cmake \
       ninja-build \
       pkg-config \
       protobuf-compiler \
       protobuf-compiler-grpc \
       libprotobuf-dev \
       libgrpc-dev \
       libgrpc++-dev \
       libboost-all-dev \
       gcc-14 \
       g++-14 \
    && rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14 \
    CXX=g++-14

WORKDIR /workspace
COPY . .

RUN cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target IPCscan_bin \
    && mkdir -p /opt/ipcscan/bin \
    && cp build/bin/IPCscan_bin /opt/ipcscan/bin/ \
    && cp -r docs /opt/ipcscan/docs

FROM arm64v8/debian:trixie AS runtime
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       libprotobuf-dev \
       libgrpc++-dev \
       libboost-system1.83.0 \
       libboost-thread1.83.0 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/ipcscan /opt/ipcscan

EXPOSE 20001
WORKDIR /opt/ipcscan/bin

ENTRYPOINT ["/opt/ipcscan/bin/IPCscan_bin"]
