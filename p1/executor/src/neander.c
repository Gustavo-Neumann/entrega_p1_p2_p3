#include "neander.h"

// Inicialização
void init_neander(Neander *neander) {
    neander->ac = 0;
    neander->pc = 0;
    neander->z = false;
    neander->n = false;
    memset(neander->memory, 0, MEMORY_SIZE); // Inicializa a memória com 0
}

// Imprimir memória
void print_memory(const Neander *neander) {
    size_t offset = 0;

    printf("AC: %03d  | PC: %03d\n", neander->ac, neander->pc);
    printf("Z : %s | N : %s\n", neander->z ? "true" : "false", neander->n ? "true" : "false");

    while (offset < MEMORY_SIZE) {
        printf("%08zx: ", offset);

        for (size_t i = 0; i < 16; i++) {
            if (offset + i < MEMORY_SIZE)
                printf("%02x ", neander->memory[offset + i]);
            else
                printf("   ");
        }

        printf("\n");
        offset += 16;
    }

    printf("\n\n");
}

// Execução
int run(Neander *neander) {
    while (neander->memory[neander->pc] != HLT) {
        // Atualiza os flags
        neander->z = (neander->ac == 0);
        neander->n = (neander->ac & 0x80) != 0;

        // Calcula o endereço uma única vez
        uint8_t address = neander->memory[neander->pc + 2];

        // Decodifica e executa a instrução
        switch (neander->memory[neander->pc]) {
            case 0x00:
                break;
            case STA:
                neander->memory[address * 2 + 4] = neander->ac;
                break;
            case LDA:
                neander->ac = neander->memory[address * 2 + 4];
                break;
            case ADD:
                neander->ac += neander->memory[address * 2 + 4];
                break;
            case 0x40:
                neander->ac = neander->memory[address * 2 + 4] | neander->ac;
                break;
            case 0x50:
                neander->ac = neander->memory[address * 2 + 4] & neander->ac;
                break;
            case NOT:
                neander->ac = ~neander->ac;
                neander->pc += 2;
                continue;
            case JMP:
                neander->pc = address * 2 + 4;
                continue;
            case JN:
                if (neander->n) {
                    neander->pc = address * 2 + 4;
                    continue;
                }
                break;
            case JZ:
                if (neander->z) {
                    neander->pc = address * 2 + 4;
                    continue;
                }
                break;
        }

        neander->pc += 4;
    }

    print_memory(neander);

    return 0;
}

int main(int argc, char const *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }

    Neander neander;
    init_neander(&neander);

    FILE *file = fopen(argv[1], "rb");
    if (!file) {
        perror("Error opening file");
        return 1;
    }

    uint8_t file_id[4];
    fread(file_id, 1, 4, file);
    fread(neander.memory + 4, 1, 512, file);
    fclose(file);

    print_memory(&neander);

    return run(&neander);
}