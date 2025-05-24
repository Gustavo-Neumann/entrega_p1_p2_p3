#ifndef PARSER_H
#define PARSER_H

#include <stdio.h>
#include "lexer.h"

#define MEMORY_SIZE 512 

typedef struct {
    char* mnemonic;
    unsigned short opcode; 
} Instruction;

typedef struct {
    Lexer* lexer;
    unsigned short memory[MEMORY_SIZE];
    int code_address;
    int max_address;
    int in_data_section;
    const Instruction* instructions;
    int num_instructions;
} Parser;

Parser* parser_init(Lexer* lexer, const Instruction* instructions, int num_instructions);
void parser_destroy(Parser* parser);
int parser_parse(Parser* parser);
int parser_write_output(Parser* parser, const char* output_filename);

int find_instruction(const Instruction* instructions, int num_instructions, const char* mnemonic);

#endif // PARSER_H