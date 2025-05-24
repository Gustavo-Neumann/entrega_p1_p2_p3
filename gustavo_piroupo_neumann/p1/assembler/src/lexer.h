#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_LINE_LENGTH 256

typedef enum {
    TOKEN_NONE,
    TOKEN_MNEMONIC,
    TOKEN_NUMBER,
    TOKEN_DIRECTIVE,
    TOKEN_EOL,
    TOKEN_EOF
} TokenType;

typedef struct {
    TokenType type;
    char* value;
    int line_number;
} Token;

typedef struct {
    FILE* input_file;
    char line[MAX_LINE_LENGTH];
    char* line_ptr;
    int line_number;
    int in_comment;
} Lexer;

Lexer* lexer_init(FILE* input);
void lexer_destroy(Lexer* lexer);
Token* lexer_next_token(Lexer* lexer);
void token_destroy(Token* token);

int is_delimiter(char c);
int is_comment(const char* line);
int is_hex(const char* str);
int hex_to_int(const char* hex);
int parse_number(const char* str);

#endif // LEXER_H