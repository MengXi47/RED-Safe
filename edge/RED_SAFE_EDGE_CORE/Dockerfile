# syntax=docker/dockerfile:1

FROM arm64v8/debian:trixie AS build
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       build-essential \
       cmake \
       ninja-build \
       pkg-config \
       protobuf-compiler \
       protobuf-compiler-grpc \
       libprotobuf-dev \
       libgrpc-dev \
       libgrpc++-dev \
       libcurl4-openssl-dev \
       gcc-14 \
       g++-14 \
    && rm -rf /var/lib/apt/lists/*

ENV CC=gcc-14 \
    CXX=g++-14

WORKDIR /workspace
COPY . .

RUN cmake -S . -B build -GNinja -DCMAKE_BUILD_TYPE=Release \
    && cmake --build build --target MQTTservice_bin \
    && mkdir -p /opt/mqttservice/bin \
    && cp build/bin/MQTTservice_bin /opt/mqttservice/bin/ \
    && if [ -d docs ]; then cp -r docs /opt/mqttservice/docs; fi

FROM arm64v8/debian:trixie AS runtime
ARG DEBIAN_FRONTEND=noninteractive

RUN apt-get update \
    && apt-get install -y --no-install-recommends \
       ca-certificates \
       libprotobuf-dev \
       libgrpc++-dev \
       libcurl4 \
    && rm -rf /var/lib/apt/lists/*

COPY --from=build /opt/mqttservice /opt/mqttservice

EXPOSE 20001
WORKDIR /opt/mqttservice/bin

ENTRYPOINT ["/opt/mqttservice/bin/MQTTservice_bin"]
