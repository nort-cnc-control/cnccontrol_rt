add_library(serial_io STATIC serial_io.c)
target_include_directories(serial_io PUBLIC .)
target_link_libraries(serial_io shell_base)
