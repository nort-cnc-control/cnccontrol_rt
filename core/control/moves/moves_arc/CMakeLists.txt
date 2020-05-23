add_library(moves_arc STATIC arc.c)

target_include_directories(moves_arc PUBLIC .)

target_link_libraries(moves_arc PUBLIC m err moves_common)

