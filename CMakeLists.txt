cmake_minimum_required(VERSION 3.10.0)
project(SANA 
        DESCRIPTION "Simulated Annealing Network Aligner"
        VERSION 2.0.0 
        LANGUAGES CXX
)

option(ENABLE_CPPCHECK "Enable static CXX checks" OFF)


if(ENABLE_CPPCHECK)
    find_program(CPPCHECK_EXECUTABLE cppcheck)
    if(NOT CPPCHECK_EXECUTABLE)
        message(FATAL_ERROR "cppcheck not found: quitting")
    else()
        message(STATUS "Found cppcheck: enabling static analysis")
    endif()
endif()


# Define cppcheck function
function(ADD_CPPCHECK TARGET_NAME)
    get_target_property(target_sources ${TARGET_NAME} SOURCES)
    set_property(
        TARGET ${TARGET_NAME}
        PROPERTY CXX_CPPCHECK
        ${CPPCHECK_EXECUTABLE}
        "--enable=warning,performance,portability,information,missingInclude"
        "--suppress=missingIncludeSystem" # https://stackoverflow.com/questions/6986033/cppcheck-cant-find-include-files
        "--std=c++11"
        "--template={file}:{line}: [{severity}][{id}] {message} {callstack}"
        "--verbose"
        "--check-config"
    )
endfunction()

enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY
    ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(src)
add_subdirectory(test)

