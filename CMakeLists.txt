cmake_minimum_required(VERSION 3.0)

project(controller)

if (NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "stm32f103")
endif ()

message("Build type = ${BUILD_TYPE}")

if (${BUILD_TYPE} STREQUAL "stm32f103")
	message("CROSS COMPILING")
	set(CMAKE_C_COMPILER "arm-none-eabi-gcc")
	set(CMAKE_CXX_COMPILER "arm-none-eabi-g++")
else()
	message("Test build")
endif()

add_subdirectory(arch/${BUILD_TYPE})
add_subdirectory(core)

add_subdirectory(drivers/ethernet_enc28j60/src/)

