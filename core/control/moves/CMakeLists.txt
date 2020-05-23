add_library(moves STATIC moves.c)

target_include_directories(moves PUBLIC .)

target_link_libraries(moves PUBLIC m err moves_line moves_arc)

add_subdirectory(moves_common)
add_subdirectory(moves_line)
add_subdirectory(moves_arc)
add_subdirectory(unit)

