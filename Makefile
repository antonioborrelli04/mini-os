CC = gcc
CFLAGS = -Wall -Wextra -pthread -Iinclude
TARGET = bin/mini-os

SRC = $(wildcard src/*.c)

all: $(TARGET)

$(TARGET): $(SRC)
	mkdir -p bin
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

run: all
	./$(TARGET)

clean:
	rm -rf bin/*