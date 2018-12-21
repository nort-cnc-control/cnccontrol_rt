PREFIX	   ?= arm-none-eabi-

CC         := $(PREFIX)gcc
CXX        := $(PREFIX)g++
LD         := $(PREFIX)gcc
AR         := $(PREFIX)ar
AS         := $(PREFIX)as
OBJCOPY    := $(PREFIX)objcopy
OBJDUMP    := $(PREFIX)objdump
GDB        := $(PREFIX)gdb

DEFS            := -DSTM32F1
FP_FLAGS        := -msoft-float -mfloat-abi=soft
ARCH_FLAGS      := -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd $(FP_FLAGS)
INCLUDE         := -I ./
CFLAGS		:= $(ARCH_FLAGS) $(INCLUDE) $(DEFS) -O2

HOST_CC         := gcc
HOST_CXX        := g++
HOST_LD         := gcc
HOST_AR         := ar
HOST_AS         := as
HOST_OBJCOPY    := objcopy
HOST_OBJDUMP    := objdump
HOST_GDB        := gdb

HOST_CFLAGS 	:= -I ./ -O0 -g -ggdb

SRCS = shell/shell.c \
	   shell/print.c \
	   gcode/gcodes.c \
	   control/line.c \
	   control/moves.c \
	   control/control.c \
	   control/planner.c \
	   math/math.c

UNIT_TESTS = unit_tests/test_gcode unit_tests/test_lines

OBJECTS_TARGET = $(addprefix build/target/, $(addsuffix .o, $(basename $(SRCS))))
OBJECTS_HOST = $(addprefix build/host/, $(addsuffix .o, $(basename $(SRCS))))

OBJECTS_UNIT = $(addsuffix .o, $(basename $(UNIT_TESTS)))

TARGET_DIR = build
HOST_DIR   = host

all: clean dirs libcore_host.a libcore_target.a unit_tests

dirs:
	mkdir -p build/host/shell
	mkdir -p build/host/gcode
	mkdir -p build/host/control
	mkdir -p build/host/math
	mkdir -p build/target/shell
	mkdir -p build/target/gcode
	mkdir -p build/target/control
	mkdir -p build/target/math

# build for target platform for running unit tests
build/target/%.o : %.c
	$(CC) -c $(CFLAGS) $< -o $@

libcore_target.a: $(OBJECTS_TARGET)
	$(AR) cr $@ $^

unit_tests: $(UNIT_TESTS)

unit_tests/% : unit_tests/%.c libcore_host.a
	$(HOST_CC) $(HOST_CFLAGS) $^ -o $@
	bash -c "$@"

# build for host platform for running unit tests
build/host/%.o : %.c
	$(HOST_CC) -c $(HOST_CFLAGS) $< -o $@

libcore_host.a: $(OBJECTS_HOST)
	$(HOST_AR) cr $@ $^


clean:
	rm -f $(UNIT_TESTS)
	rm -f $(OBJECTS_TARGET)
	rm -f $(OBJECTS_HOST)
	rm -f libcore_target.a
	rm -f libcore_host.a
