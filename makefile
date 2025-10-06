# ===============================
# Makefile for LS Project (v1.0.0)
# ===============================

CC = gcc
CFLAGS = -Wall -Wextra -g

SRC = src/lsv1.0.0.c
OBJ = obj/lsv1.0.0.o
BIN = bin/ls

# Default target
all: $(BIN)

# Rule to build the binary
$(BIN): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(BIN)

# Rule to compile the object file
obj/lsv1.0.0.o: src/lsv1.0.0.c
	@mkdir -p obj
	$(CC) $(CFLAGS) -c src/lsv1.0.0.c -o obj/lsv1.0.0.o

# Clean up
clean:
	rm -f $(OBJ) $(BIN)

.PHONY: all clean
