cmake_minimum_required(VERSION 3.5)
project(randjson C)
link_libraries(parson)
add_library(randjson randjson.c)
