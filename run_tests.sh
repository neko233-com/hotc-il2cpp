#!/bin/bash
# Quick build and test script

set -e

echo "=== hotc-il2cpp Build & Test ==="

# Check for Docker
if command -v docker &> /dev/null; then
    echo "Using Docker for build and test..."
    docker compose build
    docker compose run build-test
    echo "=== All tests passed! ==="
else
    echo "Docker not found, trying local build..."
    
    if ! command -v cmake &> /dev/null; then
        echo "Error: Neither docker nor cmake found!"
        echo "Install Docker or CMake to build."
        exit 1
    fi
    
    mkdir -p build
    cd build
    cmake .. -DCMAKE_BUILD_TYPE=Release -DHOTC_BUILD_TESTS=ON
    cmake --build . -j$(nproc 2>/dev/null || echo 4)
    ctest --output-on-failure
    echo "=== All tests passed! ==="
fi
