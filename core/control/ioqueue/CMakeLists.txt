
add_library(ioqueue STATIC print_events.c)
target_include_directories(ioqueue PUBLIC .)
target_link_libraries(ioqueue shell_print moves planner)
