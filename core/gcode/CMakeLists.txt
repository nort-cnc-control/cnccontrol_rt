add_library(gcode STATIC gcodes.c)
target_include_directories(gcode PUBLIC .)
target_link_libraries(gcode err)
