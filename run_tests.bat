@echo off
REM Quick build and test script for Windows

echo === hotc-il2cpp Build and Test ===

where docker >nul 2>&1
if %errorlevel% equ 0 (
    echo Using Docker for build and test...
    docker compose build
    docker compose run build-test
    echo === All tests passed! ===
    goto :end
)

echo Docker not found, trying local build...

where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: Neither docker nor cmake found!
    echo Install Docker Desktop or CMake to build.
    exit /b 1
)

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -DCMAKE_BUILD_TYPE=Release -DHOTC_BUILD_TESTS=ON
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

ctest --output-on-failure
if %errorlevel% neq 0 (
    echo Tests failed!
    exit /b 1
)

echo === All tests passed! ===

:end
