DEFS            += -DSTM32F1
FP_FLAGS        ?= -msoft-float -mfloat-abi=soft
ARCH_FLAGS      = -march=armv7-m -mthumb -mcpu=cortex-m3  -mfix-cortex-m3-ldrd $(FP_FLAGS)

PREFIX          ?= arm-none-eabi

CC              := $(PREFIX)-gcc
CXX             := $(PREFIX)-g++
LD              := $(PREFIX)-gcc
AR              := $(PREFIX)-ar
AS              := $(PREFIX)-as
OBJCOPY         := $(PREFIX)-objcopy
OBJDUMP         := $(PREFIX)-objdump
GDB             := $(PREFIX)-gdb

INCLUDE		+= -I ./libopencm3/include -I ./core
LIBS		+= -L ./libopencm3/lib

CFLAGS		+= $(ARCH_FLAGS) $(INCLUDE) $(DEFS) -O2
LDFLAGS		+= -T stm32.ld $(LIBS) $(ARCH_FLAGS) --static -nostartfiles


all:	main.bin

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: %.s
	$(AS) -c $< -o $@

SRC_OBJS = main.o \
	   core/shell/shell.o \
	   core/shell/print.o \
	   core/gcode/gcodes.o \
	   core/control/moves.o \
	   core/control/control.o \
	   core/control/planner.o \
	   core/math/math.o \

main.elf: $(SRC_OBJS)
	$(LD) $(LDFLAGS) $^ -lopencm3_stm32f1 -o $@


main.bin: main.elf
	$(OBJCOPY) -O binary $< $@

flash: main.bin
	stm32flash -w $< /dev/ttyUSB0

clean:
	rm main.elf main.bin $(SRC_OBJS)
