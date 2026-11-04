find_package(benchmark CONFIG QUIET)

if(NOT benchmark_FOUND)
    include(FetchContent)

    set(BENCHMARK_ENABLE_TESTING OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_GTEST_TESTS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_INSTALL_DOCS OFF CACHE BOOL "" FORCE)
    set(BENCHMARK_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)

    FetchContent_Declare(
        googlebenchmark
        URL https://github.com/google/benchmark/archive/refs/tags/v1.8.3.zip
    )
    FetchContent_MakeAvailable(googlebenchmark)
endif()

add_executable(xy_bench xy_bench.cpp)

target_compile_features(xy_bench PRIVATE cxx_std_20)
target_link_libraries(xy_bench PRIVATE aima_lib benchmark::benchmark_main)
