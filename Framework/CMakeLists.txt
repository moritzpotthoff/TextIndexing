# cmake_minimum_required(VERSION <specify CMake version here>)
project(Framework)

set(CMAKE_CXX_STANDARD 20)

if(NOT MARCH)
    set(MARCH native)
endif()

set(CMAKE_CXX_FLAGS "-Wall -Wextra -pipe")
set(CMAKE_CXX_FLAGS_DEBUG "-rdynamic -Werror -Wpedantic -pedantic-errors -Wall -Wextra -Wparentheses -Wfatal-errors -D_GLIBCXX_DEBUG -g -fno-omit-frame-pointer -O0")
set(CMAKE_CXX_FLAGS_RELEASE "-ffast-math -ftree-vectorize -Wfatal-errors -DNDEBUG -O3")

add_executable(Framework main.cpp)