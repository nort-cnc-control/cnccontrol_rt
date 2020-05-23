add_executable(test_line test_line.c)
target_link_libraries(test_line PUBLIC moves_line)


add_executable(test_arc test_arc.c)
target_link_libraries(test_arc PUBLIC moves_arc)

