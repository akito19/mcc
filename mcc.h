#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

extern char *user_input;

//
// parser.c
//

typedef struct Token Token;

typedef enum {
    TK_RESERVED, // Symbol
    TK_NUM,      // Integer
    TK_EOF,      // Token that means end of input
} TokenKind;

struct Token {
    TokenKind kind; // Type of token
    Token *next;    // Next input token
    int val;        // if kind is TK_NUM, its value
    char *str;      // Token string
    int len;        // Token length
};

extern Token *token; // Token that the process is focusing.

void error_at(char *loc, char *fmt, ...);

bool consume(char *op);
void expect(char *op);
int expect_number();

bool at_eof();
bool startswith(char *p, char *q);

Token *new_token(TokenKind kind, Token *cur, char *str, int len);
Token *tokenize();

//
// codegen.c
//


typedef struct Node Node;

// Type of AST nodes
typedef enum {
    ND_ADD, // `+`
    ND_SUB, // `-`
    ND_MUL, // `*`
    ND_DIV, // `/`
    ND_EQ,  // `==`
    ND_NE, // `!=`
    ND_LT, // `<`
    ND_LE, // `<=`
    ND_NUM, // Int
} NodeKind;

struct Node {
    NodeKind kind; // Type of Node
    Node *lhs;     // Left-Hand side
    Node *rhs;     // Right-Hand side
    int val;       // Use this value if `kind` is ND_NUM.
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);

Node *expr();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *unary();
Node *primary();
void gen(Node *node);
