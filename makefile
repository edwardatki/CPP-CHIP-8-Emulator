.DEFAULT_GOAL := all

CC = g++

CFLAGS = -c -Wall
LDFLAGS = -Wall -lsfml-graphics -lsfml-window -lsfml-system -lsfml-audio

SRC=$(wildcard *.cpp)
OBJ=$(SRC:.cpp=.o)

all: main.bin clean

run: all
	./main.bin

clean:
	rm -f *.o

$(OBJ): $(SRC)
	$(CC) $< -o $@ $(CFLAGS)

main.bin: $(OBJ)
	$(CC) $(OBJ) -o main.bin $(LDFLAGS)
