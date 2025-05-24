#ifndef NEANDER_H
#define NEANDER_H

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define STA 0x10
#define LDA 0x20
#define ADD 0x30
#define NOT 0x60
#define JMP 0x80
#define JN 0x90
#define JZ 0xA0
#define HLT 0xF0

#define MEMORY_SIZE 516

typedef struct {
    uint8_t ac;          // Acumulador
    uint8_t pc;          // Contador de programa
    bool z;              // Flag Zero
    bool n;              // Flag Negative
    uint8_t memory[MEMORY_SIZE]; // Memória
} Neander;

void init_neander(Neander *neander);
void print_memory(const Neander *neander);
int run(Neander *neander);

#endif // NEANDER_H