@echo off
REM Windows build script

echo Building hotc-il2cpp...

if not exist build mkdir build
cd build

cmake .. -G "Visual Studio 17 2022" -DHOTC_BUILD_TESTS=ON
if %errorlevel% neq 0 (
    echo CMake configuration failed!
    exit /b 1
)

cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed!
    exit /b 1
)

echo Running tests...
ctest --output-on-failure
if %errorlevel% neq 0 (
    echo Tests failed!
    exit /b 1
)

echo Done!
