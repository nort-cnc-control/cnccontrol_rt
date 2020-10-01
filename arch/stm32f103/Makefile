mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

DEFS :=

SRCS := main.c		\
	platform.c	\
	net.c

CC += -I$(ROOT)/libs/libshell/
LIBS += $(ROOT)/libs/libshell/libshell.a

ifdef CONFIG_UDP
CC += -DCONFIG_UDP_PORT=$(CONFIG_UDP_PORT)
endif

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/ -DCONFIG_LIBCORE
LIBS += $(ROOT)/core/libcore.a
SRCS += steppers.c
endif

ifdef CONFIG_LIBMODBUS
CC += -I$(ROOT)/libs/libmodbus/ -DCONFIG_LIBMODBUS
LIBS += $(ROOT)/libs/libmodbus/libmodbus.a
endif

ifdef CONFIG_UART
CC += -DCONFIG_UART
SRCS += uart.c
endif

ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
CC += -I$(ROOT)/drivers/ethernet_enc28j60/include -DCONFIG_ETHERNET_DEVICE_ENC28J60
LIBS += $(ROOT)/drivers/ethernet_enc28j60/libenc28j60.a
endif

ifdef CONFIG_ETHERNET
CC += -I$(ROOT)/libs/libeth -DCONFIG_ETHERNET
LIBS += $(ROOT)/libs/libeth/libeth.a
endif

ifdef CONFIG_IP
CC += -I$(ROOT)/libs/libip -DCONFIG_IP
LIBS += $(ROOT)/libs/libip/libip.a
endif

ifdef CONFIG_SPI
CC += -DCONFIG_SPI
SRCS += spi.c
endif

OBJS := $(SRCS:%.c=%.o)

PWD := $(shell pwd)

CC += -I$(PWD) -I$(PWD)/libopencm3/include

LDFLAGS := -T $(PWD)/stm32.ld -Wl,--gc-sections --static -nostartfiles -specs=nano.specs -specs=nosys.specs

all : controller.elf controller.bin

%.o : %.c
	$(CC) -c $< -o $@

controller.bin: controller.elf
	$(OBJCOPY) -O binary $< $@

controller.elf: libopencm3 $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) libopencm3/lib/libopencm3_stm32f1.a $(LIBS) -lm -o $@

flash: controller.bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f1x.cfg -c "init; reset halt; flash write_image erase $< 0x08000000 bin; reset; exit"

libopencm3:
	$(MAKE) -C $(mkfile_dir)/libopencm3/

clean:
	rm -f $(OBJS) controller.bin controller.elf

