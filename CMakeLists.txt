cmake_minimum_required(VERSION 3.14)

project(audio_lib VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

message(STATUS "cmake prefix path ${CMAKE_PREFIX_PATH}")
message(STATUS "cmake system prefix path ${CMAKE_SYSTEM_PREFIX_PATH}")
message(STATUS "cmake install prefix ${CMAKE_INSTALL_PREFIX}")
message(STATUS "cmake library path ${CMAKE_LIBRARY_PATH}")
message(STATUS "cmake system library path ${CMAKE_SYSTEM_LIBRARY_PATH}")

option(BUILD_TESTS "Build Tests")

add_subdirectory(src)

if (BUILD_TESTS)
    message(STATUS "Building Tests")
    add_subdirectory(test)
endif()
