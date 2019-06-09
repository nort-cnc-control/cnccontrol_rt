add_library(shell_base INTERFACE)
target_include_directories(shell_base INTERFACE .)

add_subdirectory(shell_read)
add_subdirectory(shell_print)
add_subdirectory(serial_io)
add_subdirectory(serial_rdp)
add_subdirectory(test_io)
