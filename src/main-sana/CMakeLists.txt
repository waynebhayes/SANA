set (Main-SRC
    main.cpp
    Utility.cpp
    Utility.hpp
    Definitions.hpp
)

add_executable(sana ${Main-SRC})

target_link_libraries(sana 
    lib-sana)

install(TARGETS sana DESTINATION bin)

