CC = gcc
CFLAGS = -std=gnu99 -Wall -Wextra -O2
SRC = src/lsv1.5.0.c
BIN = bin/ls

all: $(BIN)

$(BIN): $(SRC)
	mkdir -p bin obj
	$(CC) $(CFLAGS) $(SRC) -o $(BIN)

clean:
	rm -rf bin obj

.PHONY: all clean
