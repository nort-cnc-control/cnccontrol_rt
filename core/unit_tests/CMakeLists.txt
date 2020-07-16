add_executable(test_gcode test_gcode.c)
target_link_libraries(test_gcode gcode control)

add_executable(test_lines test_lines.c)
target_link_libraries(test_lines control)

add_executable(test_planner test_planner.c)
target_link_libraries(test_planner control)
