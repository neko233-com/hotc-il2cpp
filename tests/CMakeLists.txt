include(FetchContent)

FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.14.0
)

FetchContent_MakeAvailable(googletest)

add_executable(hotc-tests
    test_complete.cpp
)

target_link_libraries(hotc-tests
    PRIVATE
        hotc-il2cpp
        gtest_main
)

target_include_directories(hotc-tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

include(GoogleTest)
gtest_discover_tests(hotc-tests)
