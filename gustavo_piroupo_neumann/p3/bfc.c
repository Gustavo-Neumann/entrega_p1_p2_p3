#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <locale.h>

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
} TokenParser;

void advance_parser(TokenParser* parser);
Node* parse_assignment(TokenParser* parser);
Node* parse_expression(TokenParser* parser);
Node* parse_term(TokenParser* parser);
Node* parse_factor(TokenParser* parser);

void print_string_as_bf(const char* str);
void print_expression_as_bf(Node* node);

int main() {
    setlocale(LC_ALL, "");
    
    char input_line[1024];
    if (fgets(input_line, sizeof(input_line), stdin) == NULL) {
        return 1;
    }
    
    size_t len = strlen(input_line);
    if (len > 0 && input_line[len - 1] == '\n') {
        input_line[len - 1] = '\0';
    }
    
    TokenParser parser = {input_line, 0};
    advance_parser(&parser);
    
    Node* assignment = parse_assignment(&parser);
    if (!assignment) {
        return 1;
    }
    
    if (assignment->type == OP_NODE && assignment->operator == '=' && 
        assignment->left_child->type == VAR_NODE) {
        
        print_string_as_bf(assignment->left_child->variable);
        
        print_string_as_bf(" = ");
        
        print_expression_as_bf(assignment->right_child);
    }
    
    return 0;
}

void print_string_as_bf(const char* str) {
    const unsigned char* bytes = (const unsigned char*)str;
    
    for (int i = 0; bytes[i] != '\0'; i++) {
        unsigned char byte = bytes[i];
        
        printf("[-]");
        
        for (int j = 0; j < byte; j++) {
            printf("+");
        }
        printf(".");
    }
}

void print_expression_as_bf(Node* node) {
    if (!node) return;
    
    switch (node->type) {
        case NUM_NODE:
            {
                char num_str[20];
                sprintf(num_str, "%d", node->number);
                print_string_as_bf(num_str);
            }
            break;
            
        case VAR_NODE:
            print_string_as_bf(node->variable);
            break;
            
        case OP_NODE:
            print_expression_as_bf(node->left_child);
            
            char op_str[4];
            sprintf(op_str, " %c ", node->operator);
            print_string_as_bf(op_str);
            
            print_expression_as_bf(node->right_child);
            break;
    }
}

void advance_parser(TokenParser* parser) {
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

Node* parse_assignment(TokenParser* parser) {
    if (parser->current_char != 'I') return NULL;
    
    Node* var_node = malloc(sizeof(Node));
    var_node->type = VAR_NODE;
    strcpy(var_node->variable, parser->current_identifier);
    
    advance_parser(parser);
    
    if (parser->current_char != '=') {
        free(var_node);
        return NULL;
    }
    
    advance_parser(parser);
    
    Node* expr_node = parse_expression(parser);
    if (!expr_node) {
        free(var_node);
        return NULL;
    }
    
    Node* assign_node = malloc(sizeof(Node));
    assign_node->type = OP_NODE;
    assign_node->operator = '=';
    assign_node->left_child = var_node;
    assign_node->right_child = expr_node;
    
    return assign_node;
}

Node* parse_expression(TokenParser* parser) {
    Node* left = parse_term(parser);
    if (!left) return NULL;
    
    while (parser->current_char == '+' || parser->current_char == '-') {
        char op = parser->current_char;
        advance_parser(parser);
        
        Node* right = parse_term(parser);
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

Node* parse_term(TokenParser* parser) {
    Node* left = parse_factor(parser);
    if (!left) return NULL;
    
    while (parser->current_char == '*' || parser->current_char == '/') {
        char op = parser->current_char;
        advance_parser(parser);
        
        Node* right = parse_factor(parser);
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

Node* parse_factor(TokenParser* parser) {
    if (parser->current_char == 'N') {
        Node* num_node = malloc(sizeof(Node));
        num_node->type = NUM_NODE;
        num_node->number = parser->current_num;
        advance_parser(parser);
        return num_node;
    } else if (parser->current_char == '(') {
        advance_parser(parser);
        Node* expr = parse_expression(parser);
        if (parser->current_char == ')') {
            advance_parser(parser);
        }
        return expr;
    } else if (parser->current_char == 'I') {
        Node* var_node = malloc(sizeof(Node));
        var_node->type = VAR_NODE;
        strcpy(var_node->variable, parser->current_identifier);
        advance_parser(parser);
        return var_node;
    }
    
    return NULL;
}