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
CC += -I$(ROOT)/libmodbus/ -DCONFIG_LIBMODBUS
LIBS += $(ROOT)/libmodbus/libmodbus.a
endif

ifdef CONFIG_BOARD_MEGA2560_CONTROL_UART
CC += -DCONFIG_BOARD_MEGA2560_CONTROL_UART
endif

ifdef CONFIG_UART
CC += -DCONFIG_UART
SRCS += uart.c
endif

OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)

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

flash: controller.bin
#	avrdude -v -patmega2560 -carduino -P/dev/ttyACM0 -b115200 -D -Uflash:w:$<
	avrdude -v -patmega2560 -cwiring -P/dev/ttyACM0 -b115200 -D -Uflash:w:$<
#	avrdude -v -patmega2560 -cusbasp -b9600 -D -Uflash:w:$<

clean:
	rm -f $(OBJS) controller.bin controller.elf $(SUS)

