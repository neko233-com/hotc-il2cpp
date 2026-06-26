set(HOTC_SOURCES
    core/complete_interpreter.cpp
    core/il_parser.cpp
    core/type_system.cpp
    core/memory.cpp
    core/il2cpp_metadata.cpp
    core/exception_handling.cpp
    core/generic_sharing.cpp
    core/method_bridge.cpp
    utils/logger.cpp
)

add_library(hotc-il2cpp STATIC ${HOTC_SOURCES})

target_include_directories(hotc-il2cpp PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

if(MSVC)
    target_compile_options(hotc-il2cpp PRIVATE /W4)
else()
    target_compile_options(hotc-il2cpp PRIVATE -Wall -Wextra -Wpedantic -Wno-unused-parameter)
endif()
