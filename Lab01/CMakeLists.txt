cmake_minimum_required(VERSION 3.6)
project(Lab01)

set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(OpenMP)
if (OPENMP_FOUND)
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()

set(HEADER_FILES omp_tester.h)
set(SOURCE_FILES omp_tester.cpp main.cpp)
add_executable(Lab01 ${SOURCE_FILES})