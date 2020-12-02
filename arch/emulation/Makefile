mkfile_path := $(abspath $(lastword $(MAKEFILE_LIST)))
mkfile_dir := $(dir $(mkfile_path))

DEFS :=

ifdef CONFIG_BOARD_EMULATION_CONTROL_TCP
DEFS += -DCONFIG_TCP_PORT=$(CONFIG_TCP_PORT)
endif

ifdef CONFIG_LIBCORE
CC += -I$(ROOT)/core/
LIBCORE := $(ROOT)/core/libcore.a
endif

ifdef CONFIG_PROTECT_STACK
CC += -DCONFIG_PROTECT_STACK
endif

ifdef CONFIG_EMULATE_ENDSTOPS
CC += -DCONFIG_EMULATE_ENDSTOPS=true
else
CC += -DCONFIG_EMULATE_ENDSTOPS=false
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

