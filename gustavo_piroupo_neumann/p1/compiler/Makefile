CC = gcc
CFLAGS = -Wall -Wextra -g

# Diretórios
SRC_DIR = src
BIN_DIR = bin
OUTPUT_DIR = output
OBJ_DIR = obj

# Arquivo fonte e objeto
SRC = $(SRC_DIR)/main.c
OBJ = $(patsubst $(SRC_DIR)/%.c,$(OBJ_DIR)/%.o,$(SRC))

# Nome do executável
EXEC = $(BIN_DIR)/compilador

# Regra principal para criar o executável
all: directories $(EXEC)

# Criação dos diretórios necessários se não existirem
directories:
	@mkdir -p $(BIN_DIR)
	@mkdir -p $(OUTPUT_DIR)
	@mkdir -p $(OBJ_DIR)

# Compilação do executável
$(EXEC): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^

# Regra padrão para compilar arquivos .c em .o
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Limpeza dos arquivos gerados
clean:
	rm -f $(OBJ_DIR)/*.o
	rm -f $(EXEC)
	rm -f $(OUTPUT_DIR)/*

# Rodar o programa com a sintaxe correta
run: all
	$(EXEC) input_file.txt $(OUTPUT_DIR)/output.asm

.PHONY: all clean run directories