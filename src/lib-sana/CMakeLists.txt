file(GLOB_RECURSE lib_sources "./*cpp")
file(GLOB_RECURSE lib_headers "./*.hpp")

MACRO(HEADER_DIRECTORIES return_list)
    FILE(GLOB_RECURSE new_list *.hpp)
    SET(dir_list "")
    FOREACH(file_path ${new_list})
        GET_FILENAME_COMPONENT(dir_path ${file_path} PATH)
        SET(dir_list ${dir_list} ${dir_path})
    ENDFOREACH()
    LIST(REMOVE_DUPLICATES dir_list)
    SET(${return_list} ${dir_list})
ENDMACRO()

HEADER_DIRECTORIES(Included-headers-directories)

add_library(lib-sana ${lib_sources} ${lib_headers})

target_include_directories(lib-sana PUBLIC ${Included-headers-directories})
