# cmake_minimum_required(VERSION <specify CMake version here>)
project(codigo C)

set(CMAKE_C_STANDARD 99)

add_executable(codigo logic.c Martian_node.c)