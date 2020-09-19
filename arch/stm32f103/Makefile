mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

SRCS := main.c shell.c steps.c system.c uart.c

ifdef CONFIG_BOARD_STM32F103_CONTROL_ETHERNET
SRCS += stm32f103.eth.c
CC := $(CC) -I$(ROOT)/drivers/ethernet_enc28j60/include
LIBETH := $(ROOT)/drivers/ethernet_enc28j60/libenc28j60.a
endif

ifdef CONFIG_BOARD_STM32F103_CONTROL_SPI
SRCS += stm32f103.spi.c
endif

PWD = $(shell pwd)

CC := $(CC) -I$(PWD) -I$(PWD)/libopencm3/include

LDFLAGS := -T $(PWD)/stm32.ld -Wl,--gc-sections --static -nostartfiles -specs=nano.specs -specs=nosys.specs

OBJS := $(SRCS:%.c=%.o)

all : controller.elf controller.bin

controller.bin: controller.elf
	$(OBJCOPY) -O binary $< $@

controller.elf: libopencm3 $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) libopencm3/lib/libopencm3_stm32f1.a $(LIBCORE) $(LIBETH) -lm -o $@

%.o : %.c
	$(CC) -c $< -o $@

libopencm3:
	$(MAKE) -C $(mkfile_dir)/libopencm3/

clean:
	rm -f $(OBJS) controller.bin controller.elf

