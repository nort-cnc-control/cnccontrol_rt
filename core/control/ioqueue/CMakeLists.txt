
add_library(ioqueue STATIC print_events.c)
target_include_directories(ioqueue PUBLIC .)
target_link_libraries(ioqueue moves planner output)
