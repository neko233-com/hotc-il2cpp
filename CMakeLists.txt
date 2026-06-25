cmake_minimum_required(VERSION 3.16)
project(hotc-il2cpp VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Cross-platform settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Optimization flags per platform
if(MSVC)
    add_compile_options(/W4 /O2 /Oi /GL /Gy)
    add_link_options(/LTCG)
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(-Wall -Wextra -Wpedantic -O3 -ffast-math -funroll-loops)
    if(NOT APPLE AND NOT EMSCRIPTEN)
        add_compile_options(-march=native)
    endif()
    if(EMSCRIPTEN)
        add_compile_options(-sDISABLE_EXCEPTION_CATCHING=0)
        add_link_options(-sDISABLE_EXCEPTION_CATCHING=0)
    endif()
endif()

# Platform-specific definitions
if(EMSCRIPTEN)
    add_definitions(-DHOTC_PLATFORM_WASM=1)
elseif(WIN32)
    add_definitions(-DHOTC_PLATFORM_WINDOWS=1)
elseif(APPLE)
    add_definitions(-DHOTC_PLATFORM_MACOS=1)
    if(IOS)
        add_definitions(-DHOTC_PLATFORM_IOS=1)
    endif()
elseif(ANDROID)
    add_definitions(-DHOTC_PLATFORM_ANDROID=1)
elseif(UNIX)
    add_definitions(-DHOTC_PLATFORM_LINUX=1)
endif()

option(HOTC_BUILD_TESTS "Build tests" ON)
option(HOTC_BUILD_BENCHMARKS "Build benchmarks" ON)
option(HOTC_USE_THREADED "Enable token threaded interpreter" ON)

add_subdirectory(src)

if(HOTC_BUILD_TESTS AND NOT EMSCRIPTEN)
    enable_testing()
    add_subdirectory(tests)
endif()

if(HOTC_BUILD_BENCHMARKS AND NOT EMSCRIPTEN)
    add_subdirectory(benchmarks)
endif()
