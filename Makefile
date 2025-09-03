CC = gcc
CFLAGS = -Wall -Wextra -I./include
SRC = $(wildcard src/*.c)
BIN_DIR = build
OBJ_DIR = $(BIN_DIR)/obj
OBJ = $(patsubst src/%.c, $(OBJ_DIR)/%.o, $(SRC))
TARGET = $(BIN_DIR)/bin/templater

all: $(TARGET)

$(TARGET): $(OBJ)
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: src/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf build
