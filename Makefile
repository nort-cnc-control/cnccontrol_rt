DEFS            += -DSTM32F1
FP_FLAGS        ?= -msoft-float -mfloat-abi=soft
ARCH_FLAGS      = -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd $(FP_FLAGS)

PREFIX         ?= arm-none-eabi-

export CC              := $(PREFIX)gcc
export CXX             := $(PREFIX)g++
export LD              := $(PREFIX)gcc
export AR              := $(PREFIX)ar
export AS              := $(PREFIX)as
export OBJCOPY         := $(PREFIX)objcopy
export OBJDUMP         := $(PREFIX)objdump
export GDB             := $(PREFIX)gdb

HOST_CC         := gcc
HOST_CXX        := g++
HOST_LD         := gcc
HOST_AR         := ar
HOST_AS         := as
HOST_OBJCOPY    := objcopy
HOST_OBJDUMP    := objdump
HOST_GDB        := gdb

INCLUDE		+= -I ./libopencm3/include -I ./core
LIBS		+= -L ./libopencm3/lib

CFLAGS		+= $(ARCH_FLAGS) $(INCLUDE) $(DEFS) -O2
LDFLAGS		+= -T stm32.ld $(LIBS) $(ARCH_FLAGS) --static -nostartfiles

all:	main.bin

build/%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

build/%.o: %.s
	$(AS) -c $< -o $@

SRCS = main.c

OBJECTS_TARGET = $(addprefix build/, $(addsuffix .o, $(basename $(SRCS))))

core/libcore_target.a:
	cd core && make

main.elf: $(OBJECTS_TARGET) core/libcore_target.a
	$(LD) $(LDFLAGS) $^ -lopencm3_stm32f1 -o $@


main.bin: main.elf
	$(OBJCOPY) -O binary $< $@

flash: main.bin
	stm32flash -w $< /dev/ttyUSB0

clean:
	rm -f main.elf main.bin $(SRC_OBJS)
	cd core && make clean
