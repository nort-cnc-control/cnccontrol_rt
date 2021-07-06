FREERTOS_PATH := $(ROOT)/libs/FreeRTOS-Kernel
CC += -I$(FREERTOS_PATH)/include
CC += -I$(FREERTOS_PATH)/portable/GCC/ARM_CM4F

SRCS := shell.c

OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)

PWD := $(shell pwd)
CC += -I$(PWD)

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/ -DCONFIG_LIBCORE
endif

ifdef CONFIG_LIBMODBUS
CC += -I$(ROOT)/libmodbus/ -DCONFIG_LIBMODBUS
endif

ifdef CONFIG_ECHO
CC += -DCONFIG_ECHO
endif

ifdef CONFIG_COPY_COMMAND
CC += -DCONFIG_COPY_COMMAND
endif

all: libshell.a

libshell.a: $(OBJS)
	$(AR) rsc $@ $^

%.o: %.c
	$(CC) $< -c -o $@

clean:
	rm -f $(OBJS) libshell.a $(SUS)

