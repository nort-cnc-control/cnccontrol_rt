include(FindPkgConfig)
pkg_search_module(RDPOS REQUIRED rdpos)

add_library(rdpos_io STATIC rdpos_io.c)
target_include_directories(rdpos_io PUBLIC .)
target_include_directories(rdpos_io SYSTEM PUBLIC ${RDPOS_INCLUDE_DIRS})
target_link_libraries(rdpos_io shell_base ${RDPOS_LIBRARIES})
