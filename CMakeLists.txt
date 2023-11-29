cmake_minimum_required(VERSION 3.18)

PROJECT(jsongen)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})

add_compile_options(-O3 -flto -g3 -ansi)
include_directories("." "modules" "lib")
add_subdirectory(modules)
add_subdirectory(lib)
add_subdirectory(example)
