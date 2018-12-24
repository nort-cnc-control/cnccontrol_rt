cmake_minimum_required(VERSION 3.13)

project(controller)


set(CMAKE_C_COMPILER arm-none-eabi-gcc)
set(CMAKE_C_FLAGS "-Os -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -msoft-float -mfloat-abi=soft -nostartfiles")

include_directories(libopencm3/include/)
link_directories(libopencm3/lib/)

add_executable(controller.elf main.c stm32f103.c)
target_link_libraries(controller.elf control err gcode math shell opencm3_stm32f1)

set(LINKER_SCRIPT "stm32.ld")
set(LDSCRIPT "-T${CMAKE_SOURCE_DIR}/${LINKER_SCRIPT}")
set(LINKER_FLAGS "-specs=nosys.specs ${LDSCRIPT} -lc -lm -lnosys -Wl,--gc-sections ${STM32_PRJ_CFLAGS} ${OPT} ${USER_LINKER_FLAGS} ")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${LINKER_FLAGS}")


add_custom_target(controller.bin arm-none-eabi-objcopy -O binary controller.elf controller.bin)


add_subdirectory(core)
