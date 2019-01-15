add_library(moves STATIC common.c line.c moves.c arc.c)
target_include_directories(moves PUBLIC .)

target_link_libraries(moves PUBLIC math shell err)
