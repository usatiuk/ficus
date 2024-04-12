cmake_minimum_required(VERSION 3.18)

project(ficus C CXX ASM_NASM)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

if (TEST_MODE)
    enable_testing()
endif ()

add_subdirectory(./src/)
