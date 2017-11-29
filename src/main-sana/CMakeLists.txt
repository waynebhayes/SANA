set (Main-SRC
    main.cpp
)

add_executable(SANA ${Main-SRC})

target_link_libraries(SANA 
    lib-sana)

install(TARGETS SANA DESTINATION bin)
