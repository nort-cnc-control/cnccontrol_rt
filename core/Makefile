SRCS :=	./output/output.c					\
	./gcode/gcodes.c					\
	./control/system.c					\
	./control/control.c					\
	./control/moves/moves.c					\
	./control/moves/moves_common/common.c			\
	./control/moves/moves_common/acceleration.c		\
	./control/moves/moves_arc/segment.c			\
	./control/moves/moves_line/line.c			\
	./control/ioqueue/print_events.c			\
	./control/commands/gcode_handler/gcode_handler.c	\
	./control/commands/status/print_status.c		\
	./control/planner/planner.c				\
	./control/tools/tools.c					\
	./control/moves/moves_arc/arc.c

OBJS := $(SRCS:%.c=%.o)
TARGET := libcore.a 


TESTS_SRCS :=	./unit_tests/test_lines.c			\
		./unit_tests/test_gcode.c			\
		./unit_tests/test_planner.c			\
		./control/moves/unit/test_line.c		\
		./control/moves/unit/test_arc.c

CC := $(CC) -I./

all: $(TARGET)

%.o : %.c
	$(CC) -c $< -o $@

$(TARGET): $(OBJS)
	$(AR) rsc $@ $^

clean:
	rm -f $(OBJS) $(TARGET)

