add_library(shell_read STATIC shell_read.c)
target_include_directories(shell_read PUBLIC .)

target_link_libraries(shell_read gcode_handler shell_base)
