add_library(shell STATIC shell.c print.c)
target_include_directories(shell PUBLIC .)
