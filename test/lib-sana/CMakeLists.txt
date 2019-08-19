# This is NOT the way to construct a test library. Leaving for now
# while I refactor the actual library itself
file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

ADD_EXECUTABLE(test-lib-sana ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(test-lib-sana
    PUBLIC
        sana_lib
        gtest
        gtest_main
        # libgmock
)

add_test(NAME test-lib-sana
    COMMAND test-lib-sana)

