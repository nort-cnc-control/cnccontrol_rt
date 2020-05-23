add_library(moves_common STATIC common.c acceleration.c)

target_include_directories(moves_common PUBLIC .)

target_link_libraries(moves_common PUBLIC m err)

