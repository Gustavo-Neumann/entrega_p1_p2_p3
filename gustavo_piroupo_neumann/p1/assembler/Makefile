CC = gcc
CFLAGS = -Wall -Wextra -g

SRC_DIR = src
BIN_DIR = bin
OUTPUT_DIR = output
OBJ_DIR = obj

SRCS = $(SRC_DIR)/lexer.c $(SRC_DIR)/main.c $(SRC_DIR)/parser.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))

EXEC = $(BIN_DIR)/assembler

all: directories $(EXEC)

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(OBJ_DIR)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(EXEC)
	rm -f $(OUTPUT_DIR)/*

run: all
	$(EXEC) test_code.txt $(OUTPUT_DIR)/output.mem

.PHONY: all clean run directories