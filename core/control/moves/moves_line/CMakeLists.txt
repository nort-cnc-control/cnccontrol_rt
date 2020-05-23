add_library(moves_line STATIC line.c)

target_include_directories(moves_line PUBLIC .)

target_link_libraries(moves_line PUBLIC m err moves_common)

