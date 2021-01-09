#include "mcc.h"

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
