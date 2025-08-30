cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

find_package(audio_lib_static REQUIRED)

set(AUDIO_TEST audio_test)
set(MESSAGING_TEST messaging_test)

add_executable(${AUDIO_TEST} src/main.cpp)
add_executable(${MESSAGING_TEST} src/messaging_test.cpp)

target_link_libraries(${AUDIO_TEST} PRIVATE audio_lib_static::audio_lib_static)
target_link_libraries(${MESSAGING_TEST} PRIVATE audio_lib_static::audio_lib_static)

target_include_directories(${AUDIO_TEST} PRIVATE ${dep_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MESSAGING_TEST} PRIVATE ${dep_INCLUDE_DIR} ${CMAKE_CURRENT_SOURCE_DIR})

get_target_property(TARGET_INCLUDE_DIR ${AUDIO_TEST} INCLUDE_DIRECTORIES)
