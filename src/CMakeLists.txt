set(HOTC_SOURCES
    core/interpreter.cpp
    core/threaded.cpp
    core/il_parser.cpp
    core/type_system.cpp
    core/memory.cpp
    bridge/unity_bridge.cpp
    bridge/core_module/gameobject.cpp
    bridge/core_module/transform.cpp
    bridge/core_module/component.cpp
    utils/il_reader.cpp
    utils/logger.cpp
)

add_library(hotc-il2cpp STATIC ${HOTC_SOURCES})

target_include_directories(hotc-il2cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(HOTC_USE_THREADED)
    target_compile_definitions(hotc-il2cpp PRIVATE HOTC_USE_THREADED=1)
endif()

if(MSVC)
    target_compile_options(hotc-il2cpp PRIVATE /W4)
else()
    target_compile_options(hotc-il2cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()
