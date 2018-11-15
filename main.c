#include <stdio.h>
#include <stdlib.h>
#include "9cc.h"

void error(const Token* t){
    fprintf(stderr, "invalid character %s\n", t->input);

    exit(1);
}
void error_s(const char* msg){
    fprintf(stderr, "Error: %s\n", msg);

    exit(1);
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "arg num is invalid.\n");
        return 1;
    }

    tokenize(argv[1]);
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", 26 * 8);

    for(int i=0;i<100;++i){
        if(code[i] == NULL) break;
        gen(code[i]);
        printf("  pop rax\n");
    }

    
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    return 0;
}