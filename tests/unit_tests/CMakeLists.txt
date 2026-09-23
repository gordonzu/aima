include(GoogleTest)

add_executable(aima_unit_tests
    test_agents.cpp
)

target_compile_features(aima_unit_tests PRIVATE cxx_std_20)
target_link_libraries(aima_unit_tests PRIVATE aima_lib gtest gtest_main)

gtest_discover_tests(aima_unit_tests)
