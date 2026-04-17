cmake_minimum_required(VERSION 3.10)
project(TwinWidth)
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_compile_options(-O3)

add_subdirectory(external/googletest)
include_directories(inc)

add_executable(tests
    test/test.cpp
    src/GraphAL.cpp
    src/GraphAM.cpp
    src/MonteCarloTreeSearch_v1.cpp
    src/GreedySolver.cpp
    src/FileReader.cpp
)
target_compile_definitions(tests PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_include_directories(tests PRIVATE inc)
target_link_libraries(tests gtest gtest_main)

add_executable(main 
    src/GraphAL.cpp
    src/MonteCarloTreeSearch_v1.cpp
    src/GreedySolver.cpp
    src/FileReader.cpp
    src/main.cpp
)

add_executable(benchmark
    src/GraphAL.cpp
    src/MonteCarloTreeSearch_v1.cpp
    src/FileReader.cpp
    src/Benchmark.cpp
    src/benchmark_test.cpp)
target_compile_definitions(benchmark PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_include_directories(benchmark PRIVATE inc)

target_compile_definitions(main PRIVATE PROJECT_ROOT="${CMAKE_SOURCE_DIR}")