add_library(test_io STATIC test_io.c)
target_include_directories(test_io PUBLIC .)
target_link_libraries(test_io shell_base)
