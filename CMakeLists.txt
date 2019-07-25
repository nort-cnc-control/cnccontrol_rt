cmake_minimum_required(VERSION 3.0)

project(controller)

if (NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "stm32f103")
endif ()

add_subdirectory(arch/${BUILD_TYPE})
add_subdirectory(core)

add_subdirectory(drivers/ethernet_enc28j60/src/)

