#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_LINE_SIZE 1024
#define MAX_TOKEN_SIZE 256
#define MAX_VARIABLES 100
#define MAX_TOKENS 100
#define MAX_INSTRUCTIONS 1000

#define INITIAL_MEMORY_ADDRESS 0x80 // 80 in hexadecimal
#define TEMP_MEMORY_START 0xC8 // 200 in decimal (C8 in hex)
#define CODE_START_ADDRESS 0x00 // Endereço onde começa o código

// Tipos de tokens
typedef enum {
    TOKEN_EOF,
    TOKEN_NUMBER,
    TOKEN_VARIABLE,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_UNKNOWN
} TokenType;

// Tipos de instruções
typedef enum {
    INSTR_NOP,
    INSTR_STA,
    INSTR_LDA,
    INSTR_ADD,
    INSTR_OR,
    INSTR_AND,
    INSTR_NOT,
    INSTR_JMP,
    INSTR_JN,
    INSTR_JZ,
    INSTR_HLT
} InstructionType;

// Estrutura da instrução
typedef struct {
    InstructionType type;
    int operand;    
    int address;    
} Instruction;

// Estrutura do token
typedef struct {
    TokenType type;
    char value[MAX_TOKEN_SIZE];
    int position;
} Token;

// Estrutura para o lexer
typedef struct {
    const char *input;
    int position;
    int length;
    Token current_token;
} Lexer;

// Estrutura para variáveis
typedef struct {
    char name[MAX_TOKEN_SIZE];
    int address;
    int value;
    int initialized;
} Variable;

// Estrutura do compilador
typedef struct {
    Variable variables[MAX_VARIABLES];
    int var_count;
    int next_address;
    int temp_address;
    Instruction instructions[MAX_INSTRUCTIONS];
    int instruction_count;
    Lexer lexer;
} Compiler;

// Forward declarations para o parser recursivo descendente
int parse_expression(Compiler *c);
int parse_term(Compiler *c);
int parse_factor(Compiler *c);

// Inicialização do compilador
void init_compiler(Compiler *c) {
    c->var_count = 0;
    c->next_address = INITIAL_MEMORY_ADDRESS;
    c->temp_address = TEMP_MEMORY_START;
    c->instruction_count = 0;
}

// Adiciona uma instrução ao compilador
int add_instruction(Compiler *c, InstructionType type, int operand) {
    if (c->instruction_count >= MAX_INSTRUCTIONS) {
        fprintf(stderr, "Error: Instruction buffer overflow\n");
        return -1;
    }
    
    c->instructions[c->instruction_count].type = type;
    c->instructions[c->instruction_count].operand = operand;
    c->instructions[c->instruction_count].address = c->instruction_count;
    
    return c->instruction_count++;
}

// Modifica uma instrução existente
void modify_instruction(Compiler *c, int index, InstructionType type, int operand) {
    if (index < 0 || index >= c->instruction_count) {
        fprintf(stderr, "Error: Invalid instruction index %d\n", index);
        return;
    }
    
    c->instructions[index].type = type;
    c->instructions[index].operand = operand;
}

// Funções para gerenciar variáveis
int find_variable(Compiler *c, const char *name) {
    for (int i = 0; i < c->var_count; i++) {
        if (strcmp(c->variables[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int add_variable(Compiler *c, const char *name, int value, int initialized) {
    int index = find_variable(c, name);
    if (index >= 0) {
        // If the variable exists but wasn't initialized, update it
        if (!c->variables[index].initialized && initialized) {
            c->variables[index].value = value;
            c->variables[index].initialized = initialized;
        }
        return index;
    }
    strcpy(c->variables[c->var_count].name, name);
    c->variables[c->var_count].address = c->next_address++;
    c->variables[c->var_count].value = value;
    c->variables[c->var_count].initialized = initialized;
    return c->var_count++;
}

int add_constant(Compiler *c, int value) {
    char constant_name[MAX_TOKEN_SIZE];
    sprintf(constant_name, "_const_%d", value);
    int index = find_variable(c, constant_name);
    if (index >= 0) {
        return index;
    }
    return add_variable(c, constant_name, value, 1);
}

int get_temp_address(Compiler *c) {
    return c->temp_address++;
}

// Funções para o lexer
void init_lexer(Lexer *lexer, const char *input) {
    lexer->input = input;
    lexer->position = 0;
    lexer->length = strlen(input);
    // Inicializa com um token vazio
    lexer->current_token.type = TOKEN_UNKNOWN;
    lexer->current_token.value[0] = '\0';
    lexer->current_token.position = 0;
}

// Avança para o próximo caracter não-espaço
void skip_whitespace(Lexer *lexer) {
    while (lexer->position < lexer->length && 
           isspace(lexer->input[lexer->position])) {
        lexer->position++;
    }
}

// Obtém o próximo token
Token get_next_token(Lexer *lexer) {
    Token token;
    token.position = lexer->position;
    token.value[0] = '\0';
    
    skip_whitespace(lexer);
    
    if (lexer->position >= lexer->length) {
        token.type = TOKEN_EOF;
        return token;
    }
    
    char c = lexer->input[lexer->position];
    
    // Números (incluindo negativos)
    if (isdigit(c) || (c == '-' && lexer->position + 1 < lexer->length && 
                      isdigit(lexer->input[lexer->position + 1]))) {
        int i = 0;
        if (c == '-') {
            token.value[i++] = lexer->input[lexer->position++];
        }
        while (lexer->position < lexer->length && 
               isdigit(lexer->input[lexer->position])) {
            token.value[i++] = lexer->input[lexer->position++];
        }
        token.value[i] = '\0';
        token.type = TOKEN_NUMBER;
        return token;
    }
    
    // Identificadores (variáveis)
    if (isalpha(c) || c == '_') {
        int i = 0;
        while (lexer->position < lexer->length && 
               (isalnum(lexer->input[lexer->position]) || 
                lexer->input[lexer->position] == '_')) {
            token.value[i++] = lexer->input[lexer->position++];
        }
        token.value[i] = '\0';
        token.type = TOKEN_VARIABLE;
        return token;
    }
    
    // Operadores e parênteses
    lexer->position++;
    token.value[0] = c;
    token.value[1] = '\0';
    
    switch (c) {
        case '+': token.type = TOKEN_PLUS; break;
        case '-': token.type = TOKEN_MINUS; break;
        case '*': token.type = TOKEN_MULTIPLY; break;
        case '(': token.type = TOKEN_LPAREN; break;
        case ')': token.type = TOKEN_RPAREN; break;
        default: token.type = TOKEN_UNKNOWN; break;
    }
    
    return token;
}

// Função para avançar para o próximo token
void advance(Lexer *lexer) {
    lexer->current_token = get_next_token(lexer);
}

// Verifica se o token atual é do tipo esperado
int match(Lexer *lexer, TokenType type) {
    if (lexer->current_token.type == type) {
        advance(lexer);
        return 1;
    }
    return 0;
}

// Funções auxiliares
void clean_line(char *line) {
    char *comment = strchr(line, ';');
    if (comment) {
        *comment = '\0';
    }
    int len = strlen(line);
    while (len > 0 && isspace(line[len-1])) {
        line[--len] = '\0';
    }
}

// Gera código para carregar um valor para o acumulador
void load_accumulator(Compiler *c, int address) {
    add_instruction(c, INSTR_LDA, address);
}

// Gera código para armazenar o acumulador em um endereço
void store_accumulator(Compiler *c, int address) {
    add_instruction(c, INSTR_STA, address);
}


// Geração de código para multiplicação
void generate_multiplication(Compiler *c, int operand1_addr, int operand2_addr, int result_addr) {
    int counter_addr = get_temp_address(c);
    
    // Inicializa o resultado como 0
    int zero_idx = find_variable(c, "_zero");
    if (zero_idx < 0) {
        zero_idx = add_variable(c, "_zero", 0, 1);
    }
    load_accumulator(c, c->variables[zero_idx].address);
    store_accumulator(c, result_addr);
    
    // Inicializa o contador com operando1
    load_accumulator(c, operand1_addr);
    store_accumulator(c, counter_addr);
    
    // Início do loop de multiplicação
    int loop_start = c->instruction_count;
    
    // Verifica se o contador é zero
    load_accumulator(c, counter_addr);
    int jz_instr = add_instruction(c, INSTR_JZ, -1);
    
    // Adiciona operando2 ao resultado
    load_accumulator(c, result_addr);
    add_instruction(c, INSTR_ADD, operand2_addr);
    store_accumulator(c, result_addr);
    
    // Decrementa o contador
    load_accumulator(c, counter_addr);
    int neg_one_idx = find_variable(c, "_neg_one");
    if (neg_one_idx < 0) {
        neg_one_idx = add_variable(c, "_neg_one", 255, 1);
    }
    add_instruction(c, INSTR_ADD, c->variables[neg_one_idx].address);
    store_accumulator(c, counter_addr);
    
    // O loop_start é o índice da instrução, precisamos multiplicar por 2 pois cada
    // instrução ocupa 2 bytes no Neander (opcode + operando)
    add_instruction(c, INSTR_JMP, loop_start * 2); 
    
    // Atualiza o endereço de saída do JZ
    modify_instruction(c, jz_instr, INSTR_JZ, c->instruction_count * 2);
}

// Parser recursivo descendente
int parse_factor(Compiler *c) {
    Lexer *lexer = &c->lexer;
    int result_addr = get_temp_address(c);
    
    // Trata números
    if (lexer->current_token.type == TOKEN_NUMBER) {
        int value = atoi(lexer->current_token.value);
        int const_idx = add_constant(c, value);
        load_accumulator(c, c->variables[const_idx].address);
        store_accumulator(c, result_addr);
        advance(lexer);
    }
    // Trata variáveis
    else if (lexer->current_token.type == TOKEN_VARIABLE) {
        char var_name[MAX_TOKEN_SIZE];
        strcpy(var_name, lexer->current_token.value);
        int var_idx = find_variable(c, var_name);
        if (var_idx < 0) {
            var_idx = add_variable(c, var_name, 0, 0);
        }
        load_accumulator(c, c->variables[var_idx].address);
        store_accumulator(c, result_addr);
        advance(lexer);
    }
    // Trata expressões em parênteses
    else if (lexer->current_token.type == TOKEN_LPAREN) {
        advance(lexer); // Consome '('
        int expr_result = parse_expression(c);
        
        // Verifica se há ')' após a expressão
        if (lexer->current_token.type != TOKEN_RPAREN) {
            fprintf(stderr, "Error: Expected closing parenthesis\n");
            return result_addr; // Retorna o endereço para continuar a compilação
        }
        advance(lexer); // Consome ')'
        
        // Copia o resultado da expressão para o endereço de resultado
        load_accumulator(c, expr_result);
        store_accumulator(c, result_addr);
    }
    // Trata números negativos (como unário -factor)
    else if (lexer->current_token.type == TOKEN_MINUS) {
        advance(lexer); // Consome '-'
        
        // Parse o fator que segue o -
        int factor_addr = parse_factor(c);
        
        // Calcular o complemento de 2 para negar o valor
        load_accumulator(c, factor_addr);
        add_instruction(c, INSTR_NOT, -1);
        int one_idx = add_variable(c, "_one", 1, 1);
        add_instruction(c, INSTR_ADD, c->variables[one_idx].address);
        store_accumulator(c, result_addr);
    }
    else {
        fprintf(stderr, "Error: Unexpected token in factor\n");
        // Avança para tentar recuperar de erros
        advance(lexer);
    }
    
    return result_addr;
}

int parse_term(Compiler *c) {
    Lexer *lexer = &c->lexer;
    
    // Parse o primeiro fator
    int left_addr = parse_factor(c);
    
    // Processa * repetidamente
    while (lexer->current_token.type == TOKEN_MULTIPLY) {
        advance(lexer); // Consome o operador
        
        // Parse o próximo fator
        int right_addr = parse_factor(c);
        int result_addr = get_temp_address(c);
        
        // Gera código para multiplicação
        generate_multiplication(c, left_addr, right_addr, result_addr);
        
        // O resultado desta operação se torna o operando esquerdo para a próxima
        left_addr = result_addr;
    }
    
    return left_addr;
}

int parse_expression(Compiler *c) {
    Lexer *lexer = &c->lexer;
    
    // Parse o primeiro termo
    int left_addr = parse_term(c);
    
    // Processa + e - repetidamente
    while (lexer->current_token.type == TOKEN_PLUS || 
           lexer->current_token.type == TOKEN_MINUS) {
        TokenType op_type = lexer->current_token.type;
        advance(lexer); // Consome o operador
        
        // Parse o próximo termo
        int right_addr = parse_term(c);
        int result_addr = get_temp_address(c);
        
        // Gera código para a operação
        if (op_type == TOKEN_PLUS) {
            load_accumulator(c, left_addr);
            add_instruction(c, INSTR_ADD, right_addr);
            store_accumulator(c, result_addr);
        } else { // TOKEN_MINUS
            // Para subtração, precisamos negar o segundo operando e adicionar
            load_accumulator(c, right_addr);
            add_instruction(c, INSTR_NOT, -1);
            int one_idx = add_variable(c, "_one", 1, 1);
            add_instruction(c, INSTR_ADD, c->variables[one_idx].address);
            store_accumulator(c, right_addr);
            
            load_accumulator(c, left_addr);
            add_instruction(c, INSTR_ADD, right_addr);
            store_accumulator(c, result_addr);
        }
        
        // O resultado desta operação se torna o operando esquerdo para a próxima
        left_addr = result_addr;
    }
    
    return left_addr;
}

// Processa uma atribuição de variável
void process_assignment(Compiler *c, const char *line) {
    char var_name[MAX_TOKEN_SIZE];
    char expression[MAX_LINE_SIZE];
    
    if (sscanf(line, "%s = %[^\n]", var_name, expression) != 2) {
        fprintf(stderr, "Error parsing assignment: %s\n", line);
        return;
    }
    
    // Adiciona a variável ao compilador
    int var_idx = add_variable(c, var_name, 0, 1);
    int var_addr = c->variables[var_idx].address;
    
    // Configura o lexer para a expressão
    init_lexer(&c->lexer, expression);
    advance(&c->lexer); // Obtém o primeiro token
    
    // Parse a expressão
    int result_addr = parse_expression(c);
    
    // Armazena o resultado na variável
    load_accumulator(c, result_addr);
    store_accumulator(c, var_addr);
}

// Processa uma instrução RES
void process_res(Compiler *c, const char *line) {
    char var_name[MAX_TOKEN_SIZE];
    
    if (sscanf(line, "RES %s", var_name) != 1) {
        fprintf(stderr, "Error parsing RES statement: %s\n", line);
        return;
    }
    
    int var_idx = find_variable(c, var_name);
    if (var_idx >= 0) {
        // Carrega o valor da variável
        load_accumulator(c, c->variables[var_idx].address);
    } else {
        fprintf(stderr, "Variable not found: %s\n", var_name);
    }
}

// Converte as instruções para código assembly
void generate_assembly_code(Compiler *c, FILE *output) {
    for (int i = 0; i < c->instruction_count; i++) {
        Instruction *instr = &c->instructions[i];
        
        switch (instr->type) {
            case INSTR_NOP:
                fprintf(output, "NOP\n");
                break;
            case INSTR_STA:
                fprintf(output, "STA 0x%X\n", instr->operand);
                break;
            case INSTR_LDA:
                fprintf(output, "LDA 0x%X\n", instr->operand);
                break;
            case INSTR_ADD:
                fprintf(output, "ADD 0x%X\n", instr->operand);
                break;
            case INSTR_OR:
                fprintf(output, "OR 0x%X\n", instr->operand);
                break;
            case INSTR_AND:
                fprintf(output, "AND 0x%X\n", instr->operand);
                break;
            case INSTR_NOT:
                fprintf(output, "NOT\n");
                break;
            case INSTR_JMP:
                fprintf(output, "JMP 0x%X\n", instr->operand);
                break;
            case INSTR_JN:
                fprintf(output, "JN 0x%X\n", instr->operand);
                break;
            case INSTR_JZ:
                fprintf(output, "JZ 0x%X\n", instr->operand);
                break;
            case INSTR_HLT:
                fprintf(output, "HLT\n");
                break;
            default:
                fprintf(stderr, "Error: Unknown instruction type %d\n", instr->type);
                break;
        }
    }
}

// Gera a seção de dados
void generate_data_section(Compiler *c, FILE *output) {
    fprintf(output, ".DATA\n");
    for (int i = 0; i < c->var_count; i++) {
        fprintf(output, "0x%X 0x%X\n", c->variables[i].address, c->variables[i].value);
    }
}

// Função principal de compilação
void compile(const char *source_code, FILE *output) {
    Compiler compiler;
    init_compiler(&compiler);
    
    char lines[100][MAX_LINE_SIZE];
    int line_count = 0;
    
    char *code_copy = strdup(source_code);
    char *line = strtok(code_copy, "\n");
    
    while (line != NULL && line_count < 100) {
        clean_line(line);
        if (strlen(line) > 0) {
            strcpy(lines[line_count++], line);
        }
        line = strtok(NULL, "\n");
    }
    
    free(code_copy);
    
    // Adiciona valores constantes
    add_variable(&compiler, "_zero", 0, 1);
    add_variable(&compiler, "_one", 1, 1);
    add_variable(&compiler, "_neg_one", 255, 1); // 255 em 8 bits = -1
    
    int in_code_section = 0;
    
    // Primeira passagem - identificar variáveis
    for (int i = 0; i < line_count; i++) {
        if (strstr(lines[i], "INICIO") != NULL) {
            in_code_section = 1;
            continue;
        } else if (strstr(lines[i], "FIM") != NULL) {
            break;
        }
        
        if (in_code_section) {
            if (strchr(lines[i], '=') != NULL) {
                char var_name[MAX_TOKEN_SIZE];
                if (sscanf(lines[i], "%s =", var_name) == 1) {
                    add_variable(&compiler, var_name, 0, 0);
                }
            }
        }
    }
    
    // Segunda passagem - gerar código
    in_code_section = 0;
    for (int i = 0; i < line_count; i++) {
        if (strstr(lines[i], "INICIO") != NULL) {
            in_code_section = 1;
            continue;
        } else if (strstr(lines[i], "FIM") != NULL) {
            break;
        }
        
        if (in_code_section) {
            if (strncmp(lines[i], "RES ", 4) == 0) {
                process_res(&compiler, lines[i]);
            } else if (strchr(lines[i], '=') != NULL) {
                process_assignment(&compiler, lines[i]);
            }
        }
    }
    
    // Adiciona instrução de halt
    add_instruction(&compiler, INSTR_HLT, -1);
    
    // Gera código assembly final
    generate_data_section(&compiler, output);
    fprintf(output, ".CODE\n");
    generate_assembly_code(&compiler, output);
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <input_file> <output_file>\n", argv[0]);
        return 1;
    }
    
    FILE *input = fopen(argv[1], "r");
    if (!input) {
        fprintf(stderr, "Error opening input file: %s\n", argv[1]);
        return 1;
    }
    
    char source_code[10000] = {0};
    char buffer[MAX_LINE_SIZE];
    
    while (fgets(buffer, MAX_LINE_SIZE, input)) {
        strcat(source_code, buffer);
    }
    
    fclose(input);
    
    FILE *output = fopen(argv[2], "w");
    if (!output) {
        fprintf(stderr, "Error opening output file: %s\n", argv[2]);
        return 1;
    }
    
    compile(source_code, output);
    fclose(output);
    
    printf("Compilation completed successfully!\n");
    return 0;
}