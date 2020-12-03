include .config
export

PREFIX = $(patsubst "%",%,$(CONFIG_TOOLCHAIN_PREFIX))

CC = $(PREFIX)gcc
#CC = clang
AR = $(PREFIX)ar
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
LD = $(PREFIX)ld

ifdef CONFIG_PLATFORM_EMULATION
PLATFORM := emulation
endif

ifdef CONFIG_PLATFORM_MEGA2560
PLATFORM := mega2560
CC += -mmcu=atmega2560
endif

ifdef CONFIG_PLATFORM_STM32F103
PLATFORM := stm32f103
CC += -fdata-sections -ffunction-sections -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd -msoft-float -mfloat-abi=soft
endif

ifdef CONFIG_COMPILE_OPTIMIZATION_NONE
CC += -O0
endif

ifdef CONFIG_COMPILE_OPTIMIZATION_FOR_SIZE
CC += -Os
endif

ifdef CONFIG_PROTECT_STACK
CC += -Wl,--wrap=__stack_chk_fail
CC += -fstack-usage
endif

ifdef CONFIG_COMPILE_STACK_PROTECTION_ALL
CC += -fstack-protector-all
endif

ifdef CONFIG_COMPILE_STACK_PROTECTION_NONE
CC += -fno-stack-protector
endif

ifdef CONFIG_COMPILE_DEBUG
CC += -g -ggdb
endif

ROOT := $(shell pwd)
export ROOT

CC += -I$(ROOT)/arch/$(PLATFORM)/

export CC

all: build_board

TARGETS :=

ifdef CONFIG_LIBCORE
build_libcore:
	$(MAKE) -C core/
TARGETS += build_libcore
endif

ifdef CONFIG_LIBMODBUS
build_libmodbus:
	$(MAKE) -C libmodbus/
TARGETS += build_libmodbus
endif

build_drivers: drivers/
	$(MAKE) -C drivers/
TARGETS += build_drivers

build_libs: libs/
	$(MAKE) -C libs/
TARGETS += build_libs

build_board: $(TARGETS)
	$(MAKE) -C arch/$(PLATFORM)

flash:
	$(MAKE) -C arch/$(PLATFORM) flash

clean:
	$(MAKE) -C core clean
	$(MAKE) -C libmodbus clean
	$(MAKE) -C drivers clean
	$(MAKE) -C libs clean
	$(MAKE) -C arch/$(PLATFORM) clean

menuconfig:
	mconf Kconfig

