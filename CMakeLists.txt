cmake_minimum_required(VERSION 3.0)

project(controller)

if (NOT DEFINED BUILD_TYPE)
    set(BUILD_TYPE "target")
endif ()

if ( BUILD_TYPE STREQUAL "target")
    set(CMAKE_C_COMPILER arm-none-eabi-gcc)
    set(CMAKE_C_FLAGS "-Os -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -msoft-float -mfloat-abi=soft")

    include_directories(libopencm3/include/)
    link_directories(libopencm3/lib/)

    add_executable(controller.elf main.c stm32f103.c)
    target_link_libraries(controller.elf control err gcode shell opencm3_stm32f1 m)

    set(LINKER_FLAGS "-T ${CMAKE_SOURCE_DIR}/stm32.ld --static -nostartfiles")
    set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
    set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")

    add_custom_target(controller.bin ALL arm-none-eabi-objcopy -O binary controller.elf controller.bin)
    add_dependencies(controller.bin controller.elf)
else()
    add_executable(test test.c)
    target_link_libraries(test control err gcode m shell)
endif ()

add_subdirectory(core)

