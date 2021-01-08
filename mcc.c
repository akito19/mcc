#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef enum {
    TK_RESERVED, // Symbol
    TK_NUM,      // Integer
    TK_EOF,      // Token that means end of input
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind; // Type of token
    Token *next;    // Next input token
    int val;        // if kind is TK_NUM, its value
    char *str;      // Token string
};

// Type of AST nodes
typedef enum {
    ND_ADD, // `+`
    ND_SUB, // `-`
    ND_MUL, // `*`
    ND_DIV, // `/`
    ND_NUM, // Int
} NodeKind;

typedef struct Node Node;

struct Node {
    NodeKind kind; // Type of Node
    Node *lhs;     // Left-Hand side
    Node *rhs;     // Right-Hand side
    int val;       // Use this value if `kind` is ND_NUM.
};

// Global variables
Token *token; // Token that the process is focusing.
char *user_input;

// Function for error to report the location of error occurrence.
void error_at(char *loc, char *fmt, ...) {
    // va_list: variadic argument list
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, " "); // Output space(s) corresponding to the number of `pos`
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}


// If the next token is expected symbol, succeed the token and return True,
// Otherwise, return False.
bool consume(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        return false;
    token = token->next;
    return true;
}

// If the next token is expected symbol, succeed the token.
// Otherwise, report an error.
void expect(char op) {
    if (token->kind != TK_RESERVED || token->str[0] != op)
        error_at(token->str, "Not '%c'\n", op);
    token = token->next;
}

// If the next token is Integer, succeed the token and return its the value.
// Otherwise, report an error.
int expect_number() {
    if (token->kind != TK_NUM)
        error_at(token->str, "It's not integer.\n");
    int val = token->val;
    token = token->next;
    return val;
}

bool at_eof() {
    return token->kind == TK_EOF;
}

// Generate the new token and connect with `cur`
Token *new_token(TokenKind kind, Token *cur, char *str) {
    // `calloc` means zero-clear memory for allocated memory.
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    cur->next = tok;
    return tok;
}

// Tokenize `user_input`, and return new tokens.
Token *tokenize() {
    char *p = user_input;
    Token head;
    head.next = NULL;
    Token *cur = &head;

    while (*p) {
        // Skip spaces
        if (isspace(*p)) {
            p++;
            continue;
        }
        if (strchr("+-*/()", *p)) {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "Failed to tokenize.\n");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

//
// Generate AST nodes
//

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs  = lhs;
    node->rhs  = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val  = val;
    return node;
}

Node *expr();
Node *mul();
Node *unary();
Node *primary();

Node *expr() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume('*'))
            node = new_node(ND_MUL, node, unary());
        else if (consume('/'))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

Node *unary() {
    if (consume('+'))
        return primary();
    if (consume('-'))
        return new_node(ND_SUB, new_node_num(0), primary());
    return primary();
}

Node *primary() {
    if (consume('(')) {
        Node *node = expr();
        expect(')');
        return node;
    }

    return new_node_num(expect_number());
}

// Generate assembly code
void gen(Node *node) {
    if (node->kind == ND_NUM) {
        printf("    push %d\n", node->val);
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("    pop rdi\n");
    printf("    pop rax\n");

    switch (node->kind) {
        case ND_ADD:
            printf("    add rax, rdi\n");
            break;
        case ND_SUB:
            printf("    sub rax, rdi\n");
            break;
        case ND_MUL:
            printf("    imul rax, rdi\n");
            break;
        case ND_DIV:
            printf("    cqo\n");
            printf("    idiv rdi\n"); // idiv: signed division
            break;
    }

    printf("    push rax\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is invalid\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    Node *node = expr();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // Generate assembly code by descending AST nodes
    gen(node);

    // Load stack-top value into RAX register and return.
    printf("    pop rax\n");
    printf("    ret\n");
    return 0;
}
