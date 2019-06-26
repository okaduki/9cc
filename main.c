#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "9cc.h"

void error(const Token* t){
    fprintf(stderr, "invalid character %s\n", t->input);

    exit(1);
}
void error_s(const char* msg){
    fprintf(stderr, "Error: %s\n", msg);

    exit(1);
}


Vector* new_vector(){
    Vector *vec = (Vector*)malloc(sizeof(Vector));
    vec->capacity = 16;
    vec->data = (void**)malloc(sizeof(void*) * vec->capacity);
    vec->len = 0;

    return vec;
}

void vec_push(Vector* vec, void *data){
    if(vec->len == vec->capacity){
        vec->capacity *= 2;
        vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
    }
    vec->data[vec->len++] = data;
}


void expect(int line, int expected, int actual){
    if(expected == actual) return;
    fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
    exit(1);
}

void runtest(){
    Vector *vec = new_vector();
    expect(__LINE__, 0, vec->len);

    for(int i=0;i<100;++i)
        vec_push(vec, (void*) i);

    expect(__LINE__, 100, vec->len);
    expect(__LINE__,  0, (long)vec->data[0]);
    expect(__LINE__, 50, (long)vec->data[50]);
    expect(__LINE__, 99, (long)vec->data[99]);
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "arg num is invalid.\n");
        return 1;
    }
    if(strcmp(argv[1], "--test") == 0){
        runtest();
        return 0;
    }

    alloc_workspaces();
    tokenize(argv[1]);
    program();

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    printf("  push rbp\n");
    printf("  mov rbp, rsp\n");
    printf("  sub rsp, %d\n", 26 * 8);

    for(int i=0;i<code->len;++i){
        gen((Node*)code->data[i]);
        printf("  pop rax\n");
    }

    
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");

    return 0;
}