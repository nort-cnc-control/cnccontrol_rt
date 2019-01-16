add_library(planner STATIC planner.c)
target_include_directories(planner PUBLIC .)

target_link_libraries(planner PUBLIC moves)
