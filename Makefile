CC = gcc
CFLAGS += --std=gnu99 -pedantic -Wall
CFLAGS += -fno-strict-aliasing
CFLAGS += -D_GNU_SOURCE
CFLAGS += -D_REENTRANT
CFLAGS += -I include
CFLAGS += -g
OBJ = lockfree.o test.o

all: test

lockfree.o: lockfree.c lockfree.h list.h
	$(CC) $(CFLAGS) -c lockfree.c

test.o: test.c lockfree.h list.h
	$(CC) $(CFLAGS) -c test.c

test: $(OBJ)
	$(CC) $(CFLAGS) -o test $(OBJ)

clean:
	rm -f *.o test
