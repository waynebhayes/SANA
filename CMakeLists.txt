cmake_minimum_required(VERSION 2.8.12)
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=gnu++11")


project(SANA)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY
    ${CMAKE_HOME_DIRECTORY}/build)

enable_testing()

add_subdirectory(src)
add_subdirectory(test)

#CPPCheck

# get all project files
file(GLOB_RECURSE ALL_SOURCE_FILES src/*.cpp src/*.hpp)
string(REGEX REPLACE "CMakeFiles/[^;]+;?" "" ALL_SOURCE_FILES "${ALL_SOURCE_FILES}") 
add_custom_target(
        cppcheck
        COMMAND cppcheck
        --enable=warning,performance,portability,information,missingInclude
        --std=c++11
        --library=qt.cfg
        --template="[{severity}][{id}] {message} {callstack} \(On {file}:{line}\)"
        --verbose
        --quiet
        ${ALL_SOURCE_FILES}
)
