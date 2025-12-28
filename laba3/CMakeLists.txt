cmake_minimum_required(VERSION 3.10)
project(laba3)

set(CMAKE_C_STANDARD 11)
add_executable(child src/child.c)
add_executable(main src/parent.c)

target_link_libraries(main rt pthread)
target_link_libraries(child rt pthread)