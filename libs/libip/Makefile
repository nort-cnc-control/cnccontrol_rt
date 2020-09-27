SRCS := arp.c	\
	icmp.c	\
	ip.c	\
	udp.c

OBJS := $(SRCS:%.c=%.o)

PWD := $(shell pwd)
CC += -I$(PWD)

all: libip.a

libip.a: $(OBJS)
	$(AR) rsc $@ $^

%.o: %.c
	$(CC) $< -c -o $@

clean:
	rm -f $(OBJS) libip.a

