mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

DEFS :=

ifdef CONFIG_BOARD_EMULATION_CONTROL_TCP
DEFS += -DCONFIG_TCP_PORT=$(CONFIG_TCP_PORT)
endif

SRCS := main.c

PWD = $(shell pwd)

CC += -I$(PWD)

OBJS := $(SRCS:%.c=%.o)

all : controller.elf

controller.elf: $(OBJS)
	$(CC) $(OBJS) $(LIBCORE) -lm -lpthread -o $@

%.o : %.c
	$(CC) -c $< -o $@ $(DEFS)

clean:
	rm -f $(OBJS) controller.elf

