mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

LD := $(CC)

DEFS :=

SRCS := main.c		\
		platform.c	\
		iface.c

CC   += -I$(ROOT)/libs/libshell/
LIBS += $(ROOT)/libs/libshell/libshell.a

# ---------- FreeRTOS section -----------
FREERTOS_PATH :=        $(ROOT)/libs/FreeRTOS-Kernel
FREERTOS_SRC  :=        $(FREERTOS_PATH)/list.c                                 \
						$(FREERTOS_PATH)/queue.c                                \
                        $(FREERTOS_PATH)/tasks.c                        		\
						$(FREERTOS_PATH)/timers.c                               \
                        $(FREERTOS_PATH)/event_groups.c                         \
                        $(FREERTOS_PATH)/portable/MemMang/heap_4.c      		\
                        $(FREERTOS_PATH)/portable/GCC/ARM_CM4F/port.c
SRCS += $(FREERTOS_SRC)
CC += -I$(FREERTOS_PATH)/include
CC += -I$(FREERTOS_PATH)/portable/GCC/ARM_CM4F
# ---------- FreeRTOS section ------------

ifdef CONFIG_IP
CC += -I$(ROOT)/libs/libip -DCONFIG_IP
LIBS += $(ROOT)/libs/libip/libip.a
endif

ifdef CONFIG_UDP
CC += -DCONFIG_UDP -DCONFIG_UDP_PORT=$(CONFIG_UDP_PORT)
endif

ifdef CONFIG_PROTECT_STACK
CC += -DCONFIG_PROTECT_STACK
endif

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/ -DCONFIG_LIBCORE
LIBS += $(ROOT)/core/libcore.a
SRCS += steppers.c
endif

ifdef CONFIG_LIBMODBUS
SRCS += uart.c
CC += -I$(ROOT)/libmodbus/ -DCONFIG_LIBMODBUS
LIBS += $(ROOT)/libmodbus/libmodbus.a
endif

ifdef CONFIG_ETHERNET_DEVICE_ENC28J60
CC += -I$(ROOT)/drivers/ethernet_enc28j60/include -DCONFIG_ETHERNET_DEVICE_ENC28J60
LIBS += $(ROOT)/drivers/ethernet_enc28j60/libenc28j60.a
endif

OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)

PWD := $(shell pwd)

CC += -I$(PWD) -I$(PWD)/libopencm3/include

LDFLAGS := -T $(PWD)/stm32.ld -Wl,--gc-sections --static -nostartfiles -specs=nano.specs -specs=nosys.specs

all : controller.elf controller.bin

%.o : %.c
	$(CC) -c $< -o $@

controller.bin: controller.elf
	$(OBJCOPY) -O binary $< $@

controller.elf: libopencm3 $(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) libopencm3/lib/libopencm3_stm32f3.a $(LIBS) -o $@ -lm

flash: controller.bin
	openocd -f interface/stlink-v2.cfg -f target/stm32f3x.cfg -c "init; reset halt; flash write_image erase $< 0x08000000 bin; reset; exit"

libopencm3:
	$(MAKE) -C $(mkfile_dir)/libopencm3/

clean:
	rm -f $(OBJS) controller.bin controller.elf $(SUS)
