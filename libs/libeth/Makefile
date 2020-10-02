SRCS := ethernet.c
OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)

PWD := $(shell pwd)
CC += -I$(PWD)

all: libeth.a

libeth.a: $(OBJS)
	$(AR) rsc $@ $^

%.o: %.c
	$(CC) $< -c -o $@

clean:
	rm -f $(OBJS) libeth.a $(SUS)

