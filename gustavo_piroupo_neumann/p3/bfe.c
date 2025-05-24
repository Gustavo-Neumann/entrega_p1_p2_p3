#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#include <ctype.h>

#define MEMORY_SIZE 30000
#define PROGRAM_SIZE 1000000
#define OUTPUT_SIZE 10000

typedef struct Node Node;
struct Node {
    enum { NUM_NODE, OP_NODE, VAR_NODE } type;
    union {
        int number;
        struct {
            char operator;
            Node *left_child, *right_child;
        };
        char variable[256];
    };
};

typedef struct {
    const char* source;
    int index;
    char current_char;
    int current_num;
    char current_identifier[256];
} ExprParser;

void expr_advance_parser(ExprParser* parser);
Node* expr_parse_assignment(ExprParser* parser);
Node* expr_parse_expression(ExprParser* parser);
Node* expr_parse_term(ExprParser* parser);
Node* expr_parse_factor(ExprParser* parser);
int eval_expression(Node* node);

int main() {
    setlocale(LC_ALL, "C.UTF-8");
    
    char program[PROGRAM_SIZE];
    unsigned char memory[MEMORY_SIZE];
    unsigned char* ptr;
    char output_buffer[OUTPUT_SIZE] = {0};
    int output_pos = 0;
    int program_size = 0;
    int pc = 0;
    int c;
    
    // Inicializa memória
    memset(memory, 0, MEMORY_SIZE);
    ptr = memory;
    
    // Lê o programa Brainfuck
    while ((c = getchar()) != EOF && program_size < PROGRAM_SIZE - 1) {
        if (c == '>' || c == '<' || c == '+' || c == '-' || 
            c == '.' || c == ',' || c == '[' || c == ']') {
            program[program_size++] = c;
        }
    }
    program[program_size] = '\0';
    
    // Executa o programa e captura a saída
    while (pc < program_size) {
        switch (program[pc]) {
            case '>':
                if (ptr < memory + MEMORY_SIZE - 1) {
                    ptr++;
                } else {
                    fprintf(stderr, "Erro: Movimento além do limite superior da memória\n");
                    return 1;
                }
                break;
                
            case '<':
                if (ptr > memory) {
                    ptr--;
                } else {
                    fprintf(stderr, "Erro: Movimento além do limite inferior da memória\n");
                    return 1;
                }
                break;
                
            case '+':
                (*ptr)++;
                break;
                
            case '-':
                (*ptr)--;
                break;
                
            case '.':
                if (output_pos < OUTPUT_SIZE - 1) {
                    output_buffer[output_pos++] = *ptr;
                    output_buffer[output_pos] = '\0';
                }
                break;
                
            case ',':
                {
                    int input = getchar();
                    if (input != EOF) {
                        *ptr = (unsigned char)input;
                    } else {
                        *ptr = 0;
                    }
                }
                break;
                
            case '[':
                if (*ptr == 0) {
                    int bracket_count = 1;
                    int original_pc = pc;
                    
                    while (bracket_count > 0 && pc < program_size - 1) {
                        pc++;
                        if (program[pc] == '[') {
                            bracket_count++;
                        } else if (program[pc] == ']') {
                            bracket_count--;
                        }
                    }
                    
                    if (bracket_count > 0) {
                        fprintf(stderr, "Erro: '[' na posição %d sem ']' correspondente\n", original_pc);
                        return 1;
                    }
                }
                break;
                
            case ']':
                if (*ptr != 0) {
                    int bracket_count = 1;
                    int original_pc = pc;
                    
                    while (bracket_count > 0 && pc > 0) {
                        pc--;
                        if (program[pc] == ']') {
                            bracket_count++;
                        } else if (program[pc] == '[') {
                            bracket_count--;
                        }
                    }
                    
                    if (bracket_count > 0) {
                        fprintf(stderr, "Erro: ']' na posição %d sem '[' correspondente\n", original_pc);
                        return 1;
                    }
                }
                break;
        }
        
        pc++;
    }
    
    printf("Expressão capturada: %s\n", output_buffer);
    
    char* equals_pos = strchr(output_buffer, '=');
    if (equals_pos && equals_pos > output_buffer) {
        *equals_pos = '\0';
        char* var_name = output_buffer;
        char* expression = equals_pos + 1;
        
        while (*expression == ' ') expression++;
        
        char* end = equals_pos - 1;
        while (end > var_name && *end == ' ') {
            *end = '\0';
            end--;
        }
        
        printf("Variável: %s\n", var_name);
        printf("Expressão: %s\n", expression);
        
        ExprParser parser = {expression, 0};
        expr_advance_parser(&parser);
        
        Node* expr_ast = expr_parse_expression(&parser);
        if (expr_ast) {
            int result = eval_expression(expr_ast);
            printf("Resultado: %s = %d\n", var_name, result);
        } else {
            printf("Erro: Não foi possível parsear a expressão\n");
        }
    } else {
        printf("Saída: %s\n", output_buffer);
    }
    
    return 0;
}

void expr_advance_parser(ExprParser* parser) {
    while (parser->source[parser->index] == ' ' || 
           parser->source[parser->index] == '\t') {
        parser->index++;
    }
    
    if (parser->source[parser->index] == '\0') {
        parser->current_char = '\0';
        return;
    }
    
    unsigned char ch = (unsigned char)parser->source[parser->index];
    
    if (isalpha(ch) || ch >= 0x80) {
        int i = 0;
        while ((isalnum((unsigned char)parser->source[parser->index]) || 
                parser->source[parser->index] == '_' || 
                (unsigned char)parser->source[parser->index] >= 0x80) && 
               i < 255) {
            parser->current_identifier[i++] = parser->source[parser->index++];
        }
        parser->current_identifier[i] = '\0';
        parser->current_char = 'I';
    } else if (isdigit(ch)) {
        parser->current_num = 0;
        while (isdigit(parser->source[parser->index])) {
            parser->current_num = parser->current_num * 10 + 
                                 (parser->source[parser->index] - '0');
            parser->index++;
        }
        parser->current_char = 'N';
    } else {
        parser->current_char = parser->source[parser->index++];
    }
}

Node* expr_parse_expression(ExprParser* parser) {
    Node* left = expr_parse_term(parser);
    if (!left) return NULL;
    
    while (parser->current_char == '+' || parser->current_char == '-') {
        char op = parser->current_char;
        expr_advance_parser(parser);
        
        Node* right = expr_parse_term(parser);
        if (!right) return NULL;
        
        Node* op_node = malloc(sizeof(Node));
        op_node->type = OP_NODE;
        op_node->operator = op;
        op_node->left_child = left;
        op_node->right_child = right;
        left = op_node;
    }
    
    return left;
}

Node* expr_parse_term(ExprParser* parser) {
    Node* left = expr_parse_factor(parser);
    if (!left) return NULL;
    
    while (parser->current_char == '*' || parser->current_char == '/') {
        char op = parser->current_char;
        expr_advance_parser(parser);
        
        Node* right = expr_parse_factor(parser);
        if (!right) return NULL;
        
        Node* op_node = malloc(sizeof(Node));
        op_node->type = OP_NODE;
        op_node->operator = op;
        op_node->left_child = left;
        op_node->right_child = right;
        left = op_node;
    }
    
    return left;
}

Node* expr_parse_factor(ExprParser* parser) {
    if (parser->current_char == 'N') {
        Node* num_node = malloc(sizeof(Node));
        num_node->type = NUM_NODE;
        num_node->number = parser->current_num;
        expr_advance_parser(parser);
        return num_node;
    } else if (parser->current_char == '(') {
        expr_advance_parser(parser);
        Node* expr = expr_parse_expression(parser);
        if (parser->current_char == ')') {
            expr_advance_parser(parser);
        }
        return expr;
    } else if (parser->current_char == 'I') {
        Node* var_node = malloc(sizeof(Node));
        var_node->type = VAR_NODE;
        strcpy(var_node->variable, parser->current_identifier);
        expr_advance_parser(parser);
        return var_node;
    }
    
    return NULL;
}

int eval_expression(Node* node) {
    if (!node) return 0;
    
    switch (node->type) {
        case NUM_NODE:
            return node->number;
            
        case VAR_NODE:
            return 0;
            
        case OP_NODE:
            {
                int left = eval_expression(node->left_child);
                int right = eval_expression(node->right_child);
                
                switch (node->operator) {
                    case '+': return left + right;
                    case '-': return left - right;
                    case '*': return left * right;
                    case '/': return (right != 0) ? left / right : 0;
                    default: return 0;
                }
            }
    }
    
    return 0;
}