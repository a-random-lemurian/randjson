cmake_minimum_required(VERSION 3.18)

PROJECT(jsongen)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_compile_options(-O3 -flto -g3)
include_directories(".")

add_library(jsongen jsongen.c)
add_executable(jsongen_caller jsongen_caller.c)
target_link_libraries(jsongen_caller jsongen m)
