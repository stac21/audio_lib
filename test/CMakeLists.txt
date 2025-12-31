cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED 20)

find_package(stac_audio REQUIRED)

set(AUDIO_TEST audio_test)
set(MESSAGING_TEST messaging_test)

add_executable(${AUDIO_TEST} src/main.cpp)
add_executable(${MESSAGING_TEST} src/messaging_test.cpp)

target_link_libraries(${AUDIO_TEST} PRIVATE stac_audio::stac_audio)
target_link_libraries(${MESSAGING_TEST} PRIVATE stac_audio::stac_audio)

target_include_directories(${AUDIO_TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(${MESSAGING_TEST} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
