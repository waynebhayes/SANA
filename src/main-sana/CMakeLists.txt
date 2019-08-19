add_executable(sana_exe main.cpp Utility.cpp) 

target_link_libraries(sana_exe sana_lib)

target_compile_options(sana_exe PRIVATE -Werror)
target_compile_features(sana_exe PRIVATE cxx_std_17)

