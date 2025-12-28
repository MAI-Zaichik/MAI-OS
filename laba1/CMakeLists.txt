cmake_minimum_required(VERSION 3.10)
project(laba1)

set(CMAKE_C_STANDARD 11)
add_executable(child src/child.c)
add_executable(main src/parent.c)