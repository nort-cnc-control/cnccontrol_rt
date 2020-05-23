
add_library(gcode_handler STATIC gcode_handler.c)
target_include_directories(status PUBLIC .)
target_link_libraries(gcode_handler err gcode ioqueue status planner system)
