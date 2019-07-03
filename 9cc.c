#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "arg num is invalid.\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    char* ptr = argv[1];
    
    printf("  mov rax, %ld\n", strtol(ptr, &ptr, 10));
    while(*ptr != '\0'){
        if(*ptr == '+'){
            ++ptr;
            printf("  add rax, %ld\n", strtol(ptr, &ptr, 10));
        }
        else if(*ptr == '-'){
            ++ptr;
            printf("  sub rax, %ld\n", strtol(ptr, &ptr, 10));
        }
        else{
            fprintf(stderr, "invalid character %c\n", *ptr);
            return 1;
        }
    }

    printf("  ret\n");

    return 0;
}