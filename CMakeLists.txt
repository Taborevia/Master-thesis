cmake_minimum_required(VERSION 3.14)
project(TwinWidth)
add_compile_options(-fdiagnostics-color=always)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_compile_options(-O3)

add_subdirectory(HydraPrime)

add_library(twin_width_core OBJECT
    src/GraphAL.cpp
    src/GraphAM.cpp
    src/MonteCarloTreeSearch.cpp
    src/GreedySolver.cpp
    src/FileReader.cpp
    src/MDAdapter.cpp
)
target_compile_options(twin_width_core PRIVATE -O3)

target_compile_definitions(twin_width_core PUBLIC PROJECT_ROOT="${CMAKE_SOURCE_DIR}")
target_include_directories(twin_width_core PUBLIC inc HydraPrime)

add_executable(twin_width_solver src/mainSolver.cpp)
target_link_libraries(twin_width_solver PRIVATE twin_width_core hydraprime_core)