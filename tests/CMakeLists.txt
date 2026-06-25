find_package(GTest REQUIRED)

add_executable(hotc-tests
    test_interpreter.cpp
    test_type_system.cpp
    test_memory.cpp
)

target_link_libraries(hotc-tests
    PRIVATE
        hotc-il2cpp
        GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(hotc-tests)
