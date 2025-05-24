#include <stdio.h>
#include <stdlib.h>
#include "lexer.h"
#include "parser.h"

// Mineumonicos
Instruction instructions[] = {
    {"NOP", 0x0000},
    {"STA", 0x0010},
    {"LDA", 0x0020},
    {"ADD", 0x0030},
    {"OR",  0x0040},
    {"AND", 0x0050},
    {"NOT", 0x0060},
    {"JMP", 0x0080},
    {"JN",  0x0090},
    {"JZ",  0x00A0},
    {"HLT", 0x00F0}
};

const int NUM_INSTRUCTIONS = sizeof(instructions) / sizeof(Instruction);

int main(int argc, char* argv[]) {
    FILE* input_file;
    
    if (argc != 3) {
        fprintf(stderr, "Usage: %s input_file output_file\n", argv[0]);
        return 1;
    }
    
    input_file = fopen(argv[1], "r");
    if (!input_file) {
        fprintf(stderr, "Error opening input file: %s\n", argv[1]);
        return 1;
    }
    
    Lexer* lexer = lexer_init(input_file);
    Parser* parser = parser_init(lexer, instructions, NUM_INSTRUCTIONS);
    
    if (!parser_parse(parser)) {
        fprintf(stderr, "Error parsing input file.\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        fclose(input_file);
        return 1;
    }
    
    if (!parser_write_output(parser, argv[2])) {
        fprintf(stderr, "Error writing output file.\n");
        parser_destroy(parser);
        lexer_destroy(lexer);
        fclose(input_file);
        return 1;
    }
    
    parser_destroy(parser);
    lexer_destroy(lexer);
    fclose(input_file);
    
    return 0;
}