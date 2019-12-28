src=$(wildcard *.cpp)
obj=$(src:.cpp=.o)

CXX=g++
CXXFLAGS=-Wall -std=c++17
LDFLAGS=-lGL -lGLU -lglut -lX11 -lpthread -lpng

Atari2600Emulator: $(obj)
	$(CXX) -o $@ $^ $(LDFLAGS)

.PHONY: clean
clean:
	rm -f $(obj) Atari2600Emulator
