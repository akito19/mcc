#include "mcc.h"

Token *token;
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
bool consume(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        return false;
    token = token->next;
    return true;
}

Token *consume_ident() {
    if (token->kind != TK_IDENT)
        return NULL;
    Token *t = token;
    token = token->next;
    return t;
}

// If the next token is expected symbol, succeed the token.
// Otherwise, report an error.
void expect(char *op) {
    if (token->kind != TK_RESERVED || strlen(op) != token->len || memcmp(token->str, op, token->len))
        error_at(token->str, "Expected \"%s\"\n", op);
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

bool startswith(char *p, char *q) {
    return memcmp(p, q, strlen(q)) == 0;
}

// Generate the new token and connect with `cur`
Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
    // `calloc` means zero-clear memory for allocated memory.
    Token *tok = calloc(1, sizeof(Token));
    tok->kind = kind;
    tok->str = str;
    tok->len = len;
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
        if (startswith(p, "==")
                || startswith(p, "!=")
                || startswith(p, "<=")
                || startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }
        if ('a' <= *p && *p <= 'z') {
            cur = new_token(TK_IDENT, cur, p++, 1);
            cur->len = 1;
            continue;
        }
        if (isdigit(*p)) {
            cur = new_token(TK_NUM, cur, p, 0);
            char *q = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "Failed to tokenize.\n");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}
