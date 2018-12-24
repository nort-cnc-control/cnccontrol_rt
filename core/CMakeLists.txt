
add_library(core INTERFACE)
target_include_directories(core INTERFACE .)

add_subdirectory(err/)
add_subdirectory(control/)
add_subdirectory(gcode/)
add_subdirectory(math/)
add_subdirectory(shell/)
