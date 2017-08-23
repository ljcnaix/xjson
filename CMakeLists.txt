cmake_minimum_required (VERSION 2.6)
project (xjson_test C)

add_library(xjson xjson.c)
add_executable(xjson_test test.c)
target_link_libraries(xjson_test xjson)
