# WebGL/Emscripten cross-compilation
# Usage: emcmake cmake .. -DCMAKE_TOOLCHAIN_FILE=$EMSDK/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake

set(CMAKE_SYSTEM_NAME Emscripten)
set(CMAKE_CXX_COMPILER em++)
set(CMAKE_C_COMPILER emcc)
set(CMAKE_EXECUTABLE_SUFFIX ".js")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -s USE_PTHREADS=1")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s USE_PTHREADS=1 -s ALLOW_MEMORY_GROWTH=1")
