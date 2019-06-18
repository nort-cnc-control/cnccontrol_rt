cmake_minimum_required(VERSION 3.0)

set(LINKER_FLAGS "-T ${CMAKE_CURRENT_SOURCE_DIR}/stm32.ld --static -nostartfiles -specs=nano.specs -specs=nosys.specs")
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS "")
set(CMAKE_EXE_LINKER_FLAGS "${LINKER_FLAGS}")

include_directories(libopencm3/include/)
link_directories(libopencm3/lib/)

add_executable(controller.elf main.c stm32f103.c)
target_link_libraries(controller.elf control err gcode shell_print shell_read serial_io rdpos_io opencm3_stm32f1 m)

add_custom_target(controller.bin ALL arm-none-eabi-objcopy -O binary controller.elf controller.bin)
add_dependencies(controller.bin controller.elf)
