add_library(shell_print STATIC shell_print.c)
target_include_directories(shell_print PUBLIC .)

target_link_libraries(shell_print err shell_base)
