add_library(control STATIC control.c)
target_include_directories(control PUBLIC .)
target_link_libraries(control moves planner ioqueue gcode_handler status)

add_subdirectory(moves/)
add_subdirectory(planner/)
add_subdirectory(ioqueue/)
add_subdirectory(commands/)
