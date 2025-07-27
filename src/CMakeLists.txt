cmake_minimum_required(VERSION 3.14)

project(audio_lib VERSION 0.0.1 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

find_library(portaudio_LIBRARY_PATH portaudio)
find_library(sndfile_LIBRARY_PATH sndfile)
find_library(fftw3_LIBRARY_PATH fftw3)

message(STATUS "port audio path ${portaudio_LIBRARY_PATH}")
message(STATUS "sndfile path ${sndfile_LIBRARY_PATH}")

if (BUILD_SHARED_LIBS)
    message("Building as a shared library - yes")
    set(TARGET audio_lib)
else ()
    message("Building as a static library - yes")
    set(TARGET audio_lib_static)
endif()

# TODO figure out how to improve this set
set(dep_INCLUDE_DIR "C:/Program Files/CMake/include")

add_library(${TARGET}
    dsp_utils.hpp
    dsp_utils.cpp
    dsp_declarations.hpp
    audio_thread_data.hpp
    lock_free_queue.hpp
    message.hpp
    message.cpp
    signals.hpp
    filters.hpp
    biquad.hpp
)
### LINUX
#target_link_libraries(${TARGET} PortAudio::PortAudio)
target_link_libraries(${TARGET} ${portaudio_LIBRARY_PATH})
target_link_libraries(${TARGET} ${sndfile_LIBRARY_PATH})
target_link_libraries(${TARGET} ${fftw3_LIBRARY_PATH})
target_include_directories(${TARGET} PUBLIC ${dep_INCLUDE_DIR})
target_include_directories(${TARGET} PUBLIC .)

install(TARGETS ${TARGET})
