add_executable(test test_gcode.c)
target_link_libraries(test shell math gcode control)
