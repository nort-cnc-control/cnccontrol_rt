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

ROOT := $(shell pwd)
export ROOT

export CC

all: build_board

TARGETS :=

ifdef CONFIG_LIBCORE
build_libcore:
	$(MAKE) -C core/
TARGETS += build_libcore
endif

build_drivers: drivers/
	$(MAKE) -C drivers/
TARGETS += build_drivers

build_libs: libs/
	$(MAKE) -C libs/
TARGETS += build_libs

build_board: $(TARGETS)
	$(MAKE) -C arch/$(PLATFORM)

clean:
	$(MAKE) -C core clean
	$(MAKE) -C drivers clean
	$(MAKE) -C libs clean
	$(MAKE) -C arch/$(PLATFORM) clean

menuconfig:
	mconf Kconfig

