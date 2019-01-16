add_library(shell STATIC shell.c print.c serial_sender.c)
target_include_directories(shell PUBLIC .)
