#include "mcc.h"

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is invalid\n");
        return 1;
    }

    user_input = argv[1];
    token = tokenize();
    program();

    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // Prolog
    // To allocate spaces for 26 variables ('a' to 'z').
    printf("    push rbp\n");
    printf("    mov rbp, rsp\n");
    printf("    sub rsp, 208\n");

    // Generate assembly code from initial formula.
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // Evalueted value should be remained.
        // So, `pop` rax value in order to avoid stack overflow.
        printf("    pop rax\n");
    }

    // Epilog
    printf("    mov rsp, rbp\n");
    printf("    pop rbp\n");
    printf("    ret\n");
    return 0;
}
