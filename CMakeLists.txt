cmake_minimum_required(VERSION 3.16)
project(hotc-il2cpp VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(HOTC_BUILD_TESTS "Build tests" ON)
option(HOTC_USE_THREADED "Enable token threaded interpreter" ON)

add_subdirectory(src)

if(HOTC_BUILD_TESTS)
    enable_testing()
    add_subdirectory(tests)
endif()
