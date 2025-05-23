#include <stdlib.h>
#include <string.h>
#include "parser.h"

Parser* parser_init(Lexer* lexer, const Instruction* instructions, int num_instructions) {
    Parser* parser = (Parser*)malloc(sizeof(Parser));
    if (!parser) return NULL;
    
    parser->lexer = lexer;
    memset(parser->memory, 0, MEMORY_SIZE * sizeof(unsigned short));
    parser->code_address = 0;
    parser->max_address = 0;
    parser->in_data_section = 0;
    parser->instructions = instructions;
    parser->num_instructions = num_instructions;
    
    return parser;
}

void parser_destroy(Parser* parser) {
    if (parser) {
        free(parser);
    }
}

// Procura a instrucao pelo mineumonico
int find_instruction(const Instruction* instructions, int num_instructions, const char* mnemonic) {
    for (int i = 0; i < num_instructions; i++) {
        if (strcmp(instructions[i].mnemonic, mnemonic) == 0) {
            return i;
        }
    }
    return -1;
}

// Parse e assemble
int parser_parse(Parser* parser) {
    Token* token;
    
    while ((token = lexer_next_token(parser->lexer))->type != TOKEN_EOF) {
        // End of line
        if (token->type == TOKEN_EOL) {
            token_destroy(token);
            continue;
        }
        
        // Encontra .DATA ou .CODE
        if (token->type == TOKEN_DIRECTIVE) {
            if (strcmp(token->value, ".DATA") == 0) {
                parser->in_data_section = 1;
                token_destroy(token);
                continue;
            } else if (strcmp(token->value, ".CODE") == 0) {
                parser->in_data_section = 0;
                parser->code_address = 0;  
                token_destroy(token);
                continue;
            }
        }
        
        // Processa
        if (parser->in_data_section) {
            // .DATA
            if (token->type != TOKEN_NUMBER) {
                fprintf(stderr, "Error: Expected address number in DATA section at line %d\n", 
                        token->line_number);
                token_destroy(token);
                return 0;
            }
            
            int address = parse_number(token->value);
            token_destroy(token);
            
            token = lexer_next_token(parser->lexer);
            if (token->type != TOKEN_NUMBER) {
                fprintf(stderr, "Error: Expected value after address at line %d\n", 
                        token->line_number);
                token_destroy(token);
                return 0;
            }
            
            int value = parse_number(token->value);
            token_destroy(token);
            
            // Escreve o valor no endereco
            parser->memory[address] = value;
            
            if (address > parser->max_address) {
                parser->max_address = address;
            }
        } else {
            // .CODE
            if (token->type != TOKEN_MNEMONIC) {
                fprintf(stderr, "Error: Expected mnemonic in CODE section at line %d\n", 
                        token->line_number);
                token_destroy(token);
                return 0;
            }
            
            int instr_index = find_instruction(parser->instructions, parser->num_instructions, token->value);
            if (instr_index == -1) {
                fprintf(stderr, "Error: Unknown mnemonic '%s' at line %d\n", 
                        token->value, token->line_number);
                token_destroy(token);
                return 0;
            }
            
            parser->memory[parser->code_address] = parser->instructions[instr_index].opcode;
            
            // Verifica se precisa de operand
            if (strcmp(token->value, "HLT") != 0 && 
                strcmp(token->value, "NOT") != 0) {
                // Instrucao com operando
                token_destroy(token);
                
                token = lexer_next_token(parser->lexer);
                if (token->type != TOKEN_NUMBER) {
                    fprintf(stderr, "Error: Expected operand for instruction at line %d\n", 
                            token->line_number);
                    token_destroy(token);
                    return 0;
                }
                
                int operand = parse_number(token->value);
                parser->code_address++;
                parser->memory[parser->code_address] = operand;
            } else {
                // Nao precisa de operando para NOT ou HLT
                token_destroy(token);
            }
            
            parser->code_address++;
            
            if (parser->code_address > parser->max_address) {
                parser->max_address = parser->code_address;
            }
        }
    }
    
    token_destroy(token);
    return 1;
}

// Montagem
int parser_write_output(Parser* parser, const char* output_filename) {
    FILE* output_file;
    
    unsigned short temp_memory[MEMORY_SIZE] = {0};
    for (int i = 0; i <= parser->max_address; i++) {
        temp_memory[i] = parser->memory[i];
    }
    
    parser->memory[0] = 0x4e03;  
    parser->memory[1] = 0x5244;  
    
    for (int i = 0; i <= parser->max_address; i++) {
        parser->memory[i + 2] = temp_memory[i];
    }
    
    parser->max_address += 2;
    
    output_file = fopen(output_filename, "wb");
    if (!output_file) {
        fprintf(stderr, "Error opening output file: %s\n", output_filename);
        return 0;
    }

    fwrite(parser->memory, 2, parser->max_address + 1, output_file);
    fclose(output_file);

    char dump_filename[256];
    sprintf(dump_filename, "%s.dump", output_filename);
    FILE* dump_file = fopen(dump_filename, "w");
    if (!dump_file) {
        fprintf(stderr, "Error creating dump file: %s\n", dump_filename);
        return 0;
    }

    int max_address_rounded = (parser->max_address + 3) & ~3;
    
    if (max_address_rounded < 128) {
        max_address_rounded = 128;
    }
    

    for (int i = 0; i <= max_address_rounded; i += 4) {

        fprintf(dump_file, "%07o", i * 2); 
        
        for (int j = 0; j < 4 && i + j <= max_address_rounded; j++) {
            fprintf(dump_file, " %04x", parser->memory[i + j]);
        }
        
        fprintf(dump_file, "\n");
        
        int found_non_zero = 0;
        int k;
        for (k = i + 4; k <= max_address_rounded; k += 4) {
            for (int j = 0; j < 4 && k + j <= max_address_rounded; j++) {
                if (parser->memory[k + j] != 0) {
                    found_non_zero = 1;
                    break;
                }
            }
            if (found_non_zero) break;
        }
        
        if (!found_non_zero && i + 4 <= max_address_rounded) {
            fprintf(dump_file, "*\n");
            i = k - 4; 
        }
    }
    
    fprintf(dump_file, "%07o\n", (max_address_rounded + 1) * 2);
    
    fclose(dump_file);
    
    printf("Assembly completed successfully.\n");
    printf("Binary file generated: %s\n", output_filename);
    printf("Dump file generated: %s\n", dump_filename);
    
    return 1;
}