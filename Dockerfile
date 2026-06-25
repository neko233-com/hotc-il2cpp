FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive
ARG BUILD_TYPE=Release

RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    curl \
    pkg-config \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

COPY CMakeLists.txt ./
COPY src/ ./src/
COPY tests/ ./tests/
COPY benchmarks/ ./benchmarks/

RUN mkdir -p build && cd build \
    && cmake .. -DCMAKE_BUILD_TYPE=${BUILD_TYPE} -DHOTC_BUILD_TESTS=ON -DHOTC_BUILD_BENCHMARKS=ON \
    && cmake --build . -j$(nproc)

WORKDIR /app/build

CMD ["bash", "-c", "ctest --output-on-failure && ./benchmarks/hotc-benchmarks"]
