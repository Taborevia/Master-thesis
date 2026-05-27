cmake_minimum_required(VERSION 3.10)
project(TwinWidth)
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-O3)

add_subdirectory(HydraPrime)

add_library(twin_width_core OBJECT
    src/GraphAL.cpp
    src/GraphAM.cpp
    src/MonteCarloTreeSearch_v1.cpp
    src/MonteCarloTreeSearchExact.cpp
    src/GreedySolver.cpp
    src/FileReader.cpp
)
target_compile_options(twin_width_core PRIVATE -O3)

target_compile_definitions(twin_width_core PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_include_directories(twin_width_core PUBLIC inc)

add_executable(main src/main.cpp)
target_link_libraries(main PRIVATE twin_width_core)

add_executable(benchmark 
    src/Benchmark.cpp 
    src/benchmark_test.cpp
)
target_link_libraries(benchmark PRIVATE twin_width_core)

add_subdirectory(external/googletest)
add_executable(tests 
    test/test.cpp 
)
target_link_libraries(tests PRIVATE twin_width_core gtest gtest_main)

add_executable(twin_width_solver src/mainSolver.cpp)
target_link_libraries(twin_width_solver PRIVATE twin_width_core)

add_executable(mdadapter_test src/MDAdapterMain.cpp src/MDAdapter.cpp)
target_include_directories(mdadapter_test PRIVATE inc HydraPrime)
target_link_libraries(mdadapter_test PRIVATE hydraprime_core)
target_link_libraries(mdadapter_test PRIVATE twin_width_core)