DEFS :=

SRCS := main.c		\
	platform.c

CC += -I$(ROOT)/libs/libshell/
LIBS += $(ROOT)/libs/libshell/libshell.a

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/ -DCONFIG_LIBCORE
LIBS += $(ROOT)/core/libcore.a
SRCS += steppers.c
endif

ifdef CONFIG_LIBMODBUS
CC += -I$(ROOT)/libs/libmodbus/ -DCONFIG_LIBMODBUS
LIBS += $(ROOT)/libs/libmodbus/libmodbus.a
endif

ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
endif

ifdef CONFIG_UART
CC += -DCONFIG_UART
SRCS += uart.c
endif

OBJS := $(SRCS:%.c=%.o)

PWD := $(shell pwd)

CC += -I$(PWD) -DF_CPU=16000000UL

LDFLAGS := 

all : controller.elf controller.bin

%.o : %.c
	$(CC) -c $< -o $@

controller.bin: controller.elf
	$(OBJCOPY) -O binary $< $@

controller.elf: $(OBJS)
	$(CC) $(LDFLAGS) $(OBJS) $(LIBS) -lm -o $@

clean:
	rm -f $(OBJS) controller.bin controller.elf

