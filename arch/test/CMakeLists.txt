cmake_minimum_required(VERSION 3.0)

set(CMAKE_C_FLAGS "-O0 -DTEST_BUILD")
set(CMAKE_C_FLAGS "-O0 -DTEST_BUILD" PARENT_SCOPE)

add_executable(test main.c)
target_link_libraries(test control err gcode m pthread rt output)

