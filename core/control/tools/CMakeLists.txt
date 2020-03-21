add_library(tools STATIC tools.c)
target_include_directories(tools PUBLIC .)

target_link_libraries(tools PUBLIC err)
