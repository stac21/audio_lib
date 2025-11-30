cmake_minimum_required(VERSION 3.14)

project(audio_lib VERSION 0.0.1 LANGUAGES CXX)

find_library(fftw3_LIBRARY_PATH fftw3)

find_package(SndFile REQUIRED)
find_package(PortAudio REQUIRED)

if (BUILD_SHARED_LIBS)
    message("Building as a shared library - yes")
    set(TARGET audio_lib)
else ()
    message("Building as a static library - yes")
    set(TARGET audio_lib_static)
endif()

set(
    HEADER_FILES
        dsp_utils.hpp
        dsp_declarations.hpp
        audio_thread_data.hpp
        lock_free_queue.hpp
        message.hpp
        signals.hpp
        filters.hpp
        biquad.hpp
)

add_library(${TARGET}
    message.cpp
    ${HEADER_FILES}
)
add_library(${TARGET}::${TARGET} ALIAS ${TARGET})

set_target_properties(${TARGET}
    PROPERTIES
    CMAKE_CXX_STANDARD 20
    CMAKE_CXX_STANDARD_REQUIRED 20
)

target_link_libraries(${TARGET} PUBLIC PortAudio::portaudio)
target_link_libraries(${TARGET} PUBLIC SndFile::sndfile)
target_link_libraries(${TARGET} PUBLIC ${fftw3_LIBRARY_PATH})

target_include_directories(${TARGET}
    PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDE_DIR}>
)

get_target_property(TARGET_INCLUDE_DIR ${TARGET} INCLUDE_DIRECTORIES)

message(STATUS "Target include dir: ${TARGET_INCLUDE_DIR}")

# Include useful directory helpers for target installation
include(GNUInstallDirs)

# Configure installation destinations
install(
    TARGETS ${TARGET}
    EXPORT ${TARGET}Targets
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Configure include header install destination and files
install(
    FILES ${HEADER_FILES}
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

# Actually install the Targets file
install(
    EXPORT ${TARGET}Targets
    NAMESPACE ${TARGET}::
    FILE ${TARGET}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)

# Include the package configuration helpers for config installation
include(CMakePackageConfigHelpers)

configure_package_config_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)

install(
    FILES "${CMAKE_CURRENT_BINARY_DIR}/${TARGET}Config.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${TARGET}
)
