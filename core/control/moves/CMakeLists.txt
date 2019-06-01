add_library(moves STATIC common.c line.c arc.c moves.c acceleration.c)
target_include_directories(moves PUBLIC .)

target_link_libraries(moves PUBLIC m shell_print err)

