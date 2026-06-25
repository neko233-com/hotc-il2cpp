add_executable(hotc-benchmarks
    main.cpp
    benchmark.cpp
)

target_link_libraries(hotc-benchmarks
    PRIVATE
        hotc-il2cpp
)

target_include_directories(hotc-benchmarks PRIVATE ${CMAKE_SOURCE_DIR}/src)
