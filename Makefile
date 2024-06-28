CC = gcc
CFLAGS += --std=gnu11 -pedantic -Wall
CFLAGS += -fno-strict-aliasing
CFLAGS += -D_GNU_SOURCE
CFLAGS += -D_REENTRANT
CFLAGS += -I include
CFLAGS += -g
CFLAGS += -fsanitize=thread
LDFLAGS += -fsanitize=thread
OBJ = lockfree.o test.o

all: test

lockfree.o: lockfree.c lockfree.h list.h atomics.h
	$(CC) $(CFLAGS) -c lockfree.c

test.o: test.c lockfree.h list.h
	$(CC) $(CFLAGS) -c test.c

test: $(OBJ)
	$(CC) $(CFLAGS) -o test $(OBJ) $(LDFLAGS)

clean:
	rm -f *.o test
