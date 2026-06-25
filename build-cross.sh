# Cross-platform build script
# Supports: Windows, Linux, macOS, Android, iOS, WebGL (Emscripten)

#!/bin/bash
set -e

BUILD_TYPE=${1:-Release}
BUILD_DIR="build-${BUILD_TYPE,,}"

echo "=== hotc-il2cpp Cross-Platform Build ==="
echo "Build type: ${BUILD_TYPE}"

# Detect platform
if [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "win32" ]]; then
    PLATFORM="windows"
    GENERATOR="Visual Studio 17 2022"
elif [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macos"
    GENERATOR="Unix Makefiles"
elif [[ "$OSTYPE" == "linux-gnu"* ]]; then
    PLATFORM="linux"
    GENERATOR="Unix Makefiles"
fi

echo "Platform: ${PLATFORM}"

# Create build directory
mkdir -p "${BUILD_DIR}"
cd "${BUILD_DIR}"

# Configure
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DHOTC_BUILD_TESTS=ON \
    -DHOTC_BUILD_BENCHMARKS=ON \
    -DHOTC_USE_THREADED=ON

# Build
cmake --build . -j$(nproc 2>/dev/null || echo 4)

echo "=== Build Complete ==="
echo "Binaries in: ${BUILD_DIR}"

# Run tests if not cross-compiling
if [[ -z "${CMAKE_CROSSCOMPILING}" ]]; then
    echo "Running tests..."
    ctest --output-on-failure
fi
