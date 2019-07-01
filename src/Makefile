CC=gcc
CFLAGS=-c -Wall -g
INCLUDE=-I/usr/include/SDL

all: Atari2600Emulator.o CPU.o MMU.o TIA.o
	$(CC) $(INCLUDE) -o Atari2600Emulator Atari2600Emulator.o CPU.o MMU.o TIA.o -lSDL

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o

