SRCS :=		./output/output.c					\
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

HEADERS :=	./control/system.h					\
		./control/moves/moves_common/common.h			\
		./control/moves/moves_common/steppers.h			\
		./control/moves/moves_common/acceleration.h		\
		./control/moves/moves_arc/arc.h				\
		./control/moves/moves_arc/segment.h			\
		./control/moves/moves.h					\
		./control/moves/moves_line/line.h			\
		./control/ioqueue/print_events.h			\
		./control/control.h					\
		./control/commands/gcode_handler/gcode_handler.h	\
		./control/commands/status/print_status.h		\
		./control/planner/planner.h				\
		./control/tools/tools.h					\
		./defs.h						\
		./err/err.h						\
		./gcode/gcodes.h					\
		./output/output.h

OBJS := $(SRCS:%.c=%.o)
SUS := $(SRCS:%.c=%.su)
TARGET := libcore.a 


TESTS_SRCS :=	./unit_tests/test_lines.c			\
		./unit_tests/test_gcode.c			\
		./unit_tests/test_planner.c			\
		./control/moves/unit/test_line.c		\
		./control/moves/unit/test_arc.c

CC := $(CC) -I./

all: $(TARGET)

%.o : %.c $(HEADERS) Makefile
	$(CC) -c $< -o $@

$(TARGET): $(OBJS)
	$(AR) rsc $@ $^

clean:
	rm -f $(OBJS) $(TARGET) $(SUS)

