include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest)

target_compile_options(gtest PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-character-conversion>
)

target_compile_options(gtest_main PRIVATE
    $<$<CXX_COMPILER_ID:Clang>:-Wno-character-conversion>
)

add_subdirectory(unit_tests)
