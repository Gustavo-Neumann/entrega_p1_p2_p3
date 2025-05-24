CC = gcc
CFLAGS = -Wall -Wextra -g
SRC_DIR = src
BIN_DIR = bin
OBJ_DIR = obj
SRCS = $(SRC_DIR)/neander.c
OBJS = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRCS))
EXEC = $(BIN_DIR)/executor

all: directories $(EXEC)

directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OBJ_DIR)

$(EXEC): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJ_DIR)/neander.o: $(SRC_DIR)/neander.h

clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(EXEC)

run: all
	$(EXEC) multiplicacao.mem

.PHONY: all clean run directories