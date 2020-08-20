CFLAGS = -O
CC = g++
CXXFLAGS = -O0 -g -std=c++17 -framework OpenCL -Wall -Werror
SRC := $(wildcard *.cpp)
OBJ = $(SRC:.cpp = .o)
PROGRAM = ballpit

all: $(OBJ)
	$(CC) $(CXXFLAGS) $(OBJ) `wx-config --cxxflags --libs` -o $(PROGRAM)
clean:
	rm -f core *.o
