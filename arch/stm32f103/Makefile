mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

SRCS := main.c		\
	shell.c		\
	steppers.c	\
	platform.c	\
	spi.c		\
	net.c		\
	libip/ip.c	\
	libip/udp.c	\
	libip/arp.c	\
	libip/icmp.c

OBJS := $(SRCS:%.c=%.o)

PWD := $(shell pwd)

CC += -I$(PWD) -I$(PWD)/libip

%.o: %.c
	$(CC) $< -o $@


DEFS :=

ifdef CONFIG_UDP
DEFS += -DCONFIG_UDP_PORT=$(CONFIG_UDP_PORT)
endif

CC += -I$(ROOT)/drivers/ethernet_enc28j60/include -I$(PWD) -I$(PWD)/libopencm3/include $(DEFS)

LIBETH := $(ROOT)/drivers/ethernet_enc28j60/libenc28j60.a

LDFLAGS := -T $(PWD)/stm32.ld -Wl,--gc-sections --static -nostartfiles -specs=nano.specs -specs=nosys.specs

all : controller.elf controller.bin

%.o : %.c
	$(CC) -c $< -o $@

controller.bin: controller.elf
	$(OBJCOPY) -O binary $< $@

controller.elf: libopencm3 $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) libopencm3/lib/libopencm3_stm32f1.a $(LIBCORE) $(LIBETH) -lm -o $@

libopencm3:
	$(MAKE) -C $(mkfile_dir)/libopencm3/

clean:
	rm -f $(OBJS) controller.bin controller.elf

