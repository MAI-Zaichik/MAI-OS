cmake_minimum_required(VERSION 3.10)
project(laba4)

set(CMAKE_C_STANDARD 11)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")

include_directories(include)

add_library(lib1 SHARED src/lib1.c)
target_link_libraries(lib1 m)
set_target_properties(lib1 PROPERTIES PREFIX "" OUTPUT_NAME "lib1")

add_library(lib2 SHARED src/lib2.c)
target_link_libraries(lib2 m)
set_target_properties(lib2 PROPERTIES PREFIX "" OUTPUT_NAME "lib2")

add_executable(prog1 src/programm1.c)
target_link_libraries(prog1 lib1 m)

add_executable(prog2 src/programm2.c)
target_link_libraries(prog2 dl)