CC := $(CC) -I./

all: libmoves_common.a

%.o : %.c
	$(CC) -c $< -o $@

libmoves_common.a: common.o acceleration.o
	$(AR) rcs $@ $^
	$(MAKE) clean

clean:
	rm *.o

