SRCS := modbus.c

OBJS := $(SRCS:%.c=%.o)

PWD := $(shell pwd)
CC += -I$(PWD)

all: libmodbus.a

libmodbus.a: $(OBJS)
	$(AR) rsc $@ $^

%.o: %.c
	$(CC) $< -c -o $@

clean:
	rm -f $(OBJS) libmodbus.a

