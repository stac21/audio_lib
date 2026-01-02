cmake_minimum_required(VERSION 3.14)

project(stac_audio VERSION 0.0.1 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

option(BUILD_TESTS "Build Tests")

add_subdirectory(src)

if (BUILD_TESTS)
    message(STATUS "Building Tests")
    add_subdirectory(test)
endif()
