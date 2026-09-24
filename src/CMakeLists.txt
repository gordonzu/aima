add_library(aima_lib)

target_sources(aima_lib
    PUBLIC
        FILE_SET CXX_MODULES
        FILES
            agent.cpp
)

target_compile_features(aima_lib PUBLIC cxx_std_20)
