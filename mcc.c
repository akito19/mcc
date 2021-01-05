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


        if (*p == '+' || *p == '-') {
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

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is invalid\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // The initial of expression must be Integer.
    printf("    mov rax, %d\n", expect_number());

    while (!at_eof()) {
        if (consume('+')) {
            printf("    add rax, %d\n", expect_number());
            continue;
        }

        expect('-');
        printf(" sub rax, %d\n", expect_number());
    }

    printf("    ret\n");
    return 0;
}
