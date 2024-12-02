cmake_minimum_required(VERSION 3.14)

project(audio_lib VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

message(STATUS ${CMAKE_PREFIX_PATH})
message(STATUS ${CMAKE_SYSTEM_PREFIX_PATH})
message(STATUS ${CMAKE_INSTALL_PREFIX})

option(BUILD_SHARED_LIBS "Build as a shared library")
option(BUILD_STATIC_LIBS "Build as a static library")
option(INSTALL_STATIC_LIBS "Install static library")

if (NOT BUILD_SHARED_LIBS AND NOT BUILD_STATIC_LIBS)
    message(FATAL_ERROR "One or both of BUILD_SHARED_LIBS or BUILD_STATIC_LIBS must be set to ON to build this library")
endif()

### LINUX
# find_package(portaudio)

# if (portaudio_FOUND)
#     message(STATUS "PortAudio version: " ${portaudio_VERSION})
# else()
#     message(FATAL_ERROR "PortAudio NOT found!")
# endif()

set(SOURCE_FILES
    src/dsp_utils.cpp
    src/message.cpp
)

find_library(portaudio_LIBRARY_PATH portaudio)
find_library(sndfile_LIBRARY_PATH sndfile)

message(STATUS ${portaudio_LIBRARY_PATH})

set(TARGET audio_lib)

# TODO figure out how to improve this set
set(dep_INCLUDE_DIR "C:/Program Files/CMake/include")

add_library(${TARGET} ${SOURCE_FILES})
### LINUX
#target_link_libraries(${TARGET} PortAudio::PortAudio)
target_link_libraries(${TARGET} ${portaudio_LIBRARY_PATH})
target_link_libraries(${TARGET} ${sndfile_LIBRARY_PATH})
target_include_directories(${TARGET} PRIVATE ${dep_INCLUDE_DIR})
target_include_directories(${TARGET} PRIVATE .)


#set(INSTALL_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake/audio_lib)
set(INSTALL_DIR bin)

install(TARGETS ${target} DESTINATION ${INSTALL_DIR})

if (BUILD_SHARED_LIBS)
    message(STATUS "Building as a shared library - yes")
else ()
    message(STATUS "Building as a shared library - no")
endif()

if (BUILD_STATIC_LIBS)
    message(STATUS "Building as a static library - yes")
else ()
    message(STATUS "Building as a static library - no")
endif()