CC=gcc
CFLAGS=-std=c99 -Wall -pedantic -g3

all: testShips

testShips: testShips.o ships.o
	$(CC) $(CFLAGS) -o $@ $^

testShips.o: testShips.c ships.h

ships.o: ships2.c ships.h

clean: $(RM) testShips *.o
