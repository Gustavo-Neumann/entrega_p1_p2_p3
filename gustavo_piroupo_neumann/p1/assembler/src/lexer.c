#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"

Lexer* lexer_init(FILE* input) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (!lexer) return NULL;
    
    lexer->input_file = input;
    lexer->line[0] = '\0';
    lexer->line_ptr = lexer->line;
    lexer->line_number = 0;
    lexer->in_comment = 0;
    
    return lexer;
}

void lexer_destroy(Lexer* lexer) {
    if (lexer) {
        free(lexer);
    }
}

// Verifica se é delimitador
int is_delimiter(char c) {
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r' || c == ';');
}

// Verifica se é comentario
int is_comment(const char* line) {
    // Pula espacos em branco
    while (*line && (*line == ' ' || *line == '\t')) {
        line++;
    }
    
    // Se a linha começa com ;
    return  (*line == ';');
}

// Verifica se é hexadecimal
int is_hex(const char* str) {

    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        str += 2; 
    }
    
    while (*str) {
        if (!((*str >= '0' && *str <= '9') || 
              (*str >= 'a' && *str <= 'f') || 
              (*str >= 'A' && *str <= 'F'))) {
            return 0; 
        }
        str++;
    }
    
    return 1; 
}

int hex_to_int(const char* hex) {
    int result = 0;
    while (*hex) {
        result *= 16;
        if (*hex >= '0' && *hex <= '9') {
            result += *hex - '0';
        } else if (*hex >= 'A' && *hex <= 'F') {
            result += *hex - 'A' + 10;
        } else if (*hex >= 'a' && *hex <= 'f') {
            result += *hex - 'a' + 10;
        }
        hex++;
    }
    return result;
}

// Converte string pra numero
int parse_number(const char* str) {
    if (str[0] == '0' && (str[1] == 'x' || str[1] == 'X')) {
        return strtol(str + 2, NULL, 16);
    }
    
    if (is_hex(str)) {
        return hex_to_int(str);
    }
    
    return atoi(str);
}

// Pega o proximo token
Token* lexer_next_token(Lexer* lexer) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (!token) return NULL;
    
    token->type = TOKEN_NONE;
    token->value = NULL;
    token->line_number = lexer->line_number;
    
    // Se a linha acabar le uma nova
    if (*(lexer->line_ptr) == '\0') {
        if (fgets(lexer->line, MAX_LINE_LENGTH, lexer->input_file) == NULL) {
            token->type = TOKEN_EOF;
            return token;
        }
        
        lexer->line_ptr = lexer->line;
        lexer->line_number++;
        
        // Pula comentarios
        if (is_comment(lexer->line)) {
            free(token);
            return lexer_next_token(lexer);
        }
        
        token->line_number = lexer->line_number;
    }
    
    // Pula delimitadores
    while (*(lexer->line_ptr) && is_delimiter(*(lexer->line_ptr))) {
        if (*(lexer->line_ptr) == ';') {
            *(lexer->line_ptr) = '\0';
            token->type = TOKEN_EOL;
            return token;
        }
        (lexer->line_ptr)++;
    }
    
    // Verifica fim da linha
    if (*(lexer->line_ptr) == '\0') {
        token->type = TOKEN_EOL;
        return token;
    }
    
    // Comeco e fim do token
    char* token_start = lexer->line_ptr;
    
    while (*(lexer->line_ptr) && !is_delimiter(*(lexer->line_ptr))) {
        (lexer->line_ptr)++;
    }
    
    char saved_char = *(lexer->line_ptr);
    if (*(lexer->line_ptr)) {
        *(lexer->line_ptr) = '\0';
        (lexer->line_ptr)++;
    }
    
    // Determina o tipo do token
    if (token_start[0] == '.') {
        token->type = TOKEN_DIRECTIVE;
    } else if (isdigit(token_start[0]) || 
              (token_start[0] == '0' && (token_start[1] == 'x' || token_start[1] == 'X'))) {
        token->type = TOKEN_NUMBER;
    } else {
        token->type = TOKEN_MNEMONIC;
    }
    
    token->value = strdup(token_start);

    if (saved_char != '\0') {
        *(lexer->line_ptr - 1) = saved_char;
    }
    
    return token;
}

void token_destroy(Token* token) {
    if (token) {
        if (token->value) {
            free(token->value);
        }
        free(token);
    }
}