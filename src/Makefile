CC=gcc
CFLAGS=-c -Wall -g
INCLUDE=-I/usr/include/X11 -I/usr/include/GL
LINK=-lX11 -lGL

all: Atari2600Emulator.o CPU.o MMU.o TIA.o
	$(CC) $(INCLUDE) -o Atari2600Emulator Atari2600Emulator.o CPU.o MMU.o TIA.o $(LINK)

.c.o:
	$(CC) $(CFLAGS) $<

clean:
	rm -rf *.o

