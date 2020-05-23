add_library(system STATIC system.c)
target_include_directories(system PUBLIC .)

add_library(control STATIC control.c)
target_include_directories(control PUBLIC .)
target_link_libraries(control moves planner ioqueue gcode_handler status system)

add_subdirectory(moves/)
add_subdirectory(tools/)
add_subdirectory(planner/)
add_subdirectory(ioqueue/)
add_subdirectory(commands/)
