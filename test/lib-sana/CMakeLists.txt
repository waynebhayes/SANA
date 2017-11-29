file(GLOB_RECURSE SOURCES "*.cpp")
file(GLOB_RECURSE HEADERS "*.hpp")

ADD_EXECUTABLE(test-lib-sana ${HEADERS} ${SOURCES})

TARGET_LINK_LIBRARIES(test-lib-sana
    lib-sana
    libgtest
    libgmock
)

add_test(NAME test-lib-sana
    COMMAND test-lib-sana)
