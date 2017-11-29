cmake_minimum_required(VERSION 3.4.3)

project(SANA)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}/build)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)
