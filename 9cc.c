#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum {
    TK_NUM = 256,
    TK_EOF,
};

typedef struct {
    int type;
    int val;
    char* input;
} Token;

Token tokens[100];
void error(const Token* t){
    fprintf(stderr, "invalid character %s\n", t->input);

    exit(1);
}

void tokenize(char* p){
    int i = 0;
    while(1){
        while(isspace(*p)) ++p;
        if(*p == '\0'){
            tokens[i].type = TK_EOF;
            return;
        }

        if(*p == '+' || *p == '-'){
            tokens[i].type = *p;
            tokens[i].input = p;
            ++i;
            ++p;
            continue;
        }

        if(isdigit(*p)){
            tokens[i].type = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);

            ++i;
            continue;
        }

        Token token;
        token.input = p;
        error(&token);
    }
}

int main(int argc, char **argv){
    if(argc != 2){
        fprintf(stderr, "arg num is invalid.\n");
        return 1;
    }

    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    tokenize(argv[1]);

    if(tokens[0].type != TK_NUM){
        error(&tokens[0]);
    }

    int i = 1;
    printf("  mov rax, %d\n", tokens[0].val);
    while(tokens[i].type != TK_EOF){
        if(tokens[i].type == '+'){
            if(tokens[i+1].type != TK_NUM)
                error(&tokens[i+1]);
            printf("  add rax, %d\n", tokens[i+1].val);
            i += 2;
        }
        else if(tokens[i].type == '-'){
            if(tokens[i+1].type != TK_NUM)
                error(&tokens[i+1]);
            printf("  sub rax, %d\n", tokens[i+1].val);
            i += 2;
        }
        else{
            error(&tokens[i]);
        }
    }

    printf("  ret\n");

    return 0;
}