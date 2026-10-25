cmake_minimum_required(VERSION 3.14)

# Avoid warning about DOWNLOAD_EXTRACT_TIMESTAMP in CMake 3.24:
if (CMAKE_VERSION VERSION_GREATER_EQUAL "3.24.0")
  cmake_policy(SET CMP0135 NEW)
endif()

# options
option(BUILD_TESTS "Build test programs" OFF)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# source files
file(GLOB_RECURSE MAIN_SRC
  ds/*.[ch]pp
  modular/*.[ch]pp
  readwrite/*.[ch]pp
  util/*.[ch]pp
)

include_directories(
  .
)

# project
project(tww)

# optimize kissat for UNsat instances
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -DNDEBUG -DUNSAT -DNPROOFS -DQUIET")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -DNDEBUG -Wall -funroll-loops -fno-stack-limit -O3")

# build options
add_compile_definitions(PROFILE_ON=false)
add_compile_definitions(TRACE_ON=false)
add_compile_definitions(LOGGING_ON=false)

# static library hydraprime_core
add_library(hydraprime_core STATIC ${MAIN_SRC} ${KISSAT_SRC})
target_include_directories(hydraprime_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(hydraprime_core PUBLIC PROFILE_ON=false TRACE_ON=false LOGGING_ON=false)
