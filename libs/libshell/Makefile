SRCS := shell.c

OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)

PWD := $(shell pwd)
CC += -I$(PWD)

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/ -DCONFIG_LIBCORE
endif

ifdef CONFIG_LIBMODBUS
CC += -I$(ROOT)/libs/libmodbus/ -DCONFIG_LIBMODBUS
endif

all: libshell.a

libshell.a: $(OBJS)
	$(AR) rsc $@ $^

%.o: %.c
	$(CC) $< -c -o $@

clean:
	rm -f $(OBJS) libshell.a $(SUS)

