include .config
export

PREFIX = $(patsubst "%",%,$(CONFIG_TOOLCHAIN_PREFIX))

CC = $(PREFIX)gcc
AR = $(PREFIX)ar
OBJCOPY = $(PREFIX)objcopy
OBJDUMP = $(PREFIX)objdump
LD = $(PREFIX)ld

ifdef CONFIG_PLATFORM_EMULATION
PLATFORM := emulation
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

ifdef CONFIG_COMPILE_DEBUG
CC += -g -ggdb
endif

CC += -I$(shell pwd)/core
export CC

ROOT := $(shell pwd)
export ROOT

LIBCORE := $(ROOT)/core/libcore.a
export LIBCORE


all: build_board

build_libcore:
	$(MAKE) -C core/

build_drivers: drivers/
	$(MAKE) -C drivers/

build_board: build_libcore build_drivers
	$(MAKE) -C arch/$(PLATFORM)

clean:
	$(MAKE) -C core clean
	$(MAKE) -C drivers clean
	$(MAKE) -C arch/$(PLATFORM) clean

menuconfig:
	mconf Kconfig

