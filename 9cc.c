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

enum {
    ND_NUM = 256,
};

typedef struct Node {
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int val;
} Node;

Token tokens[100];
void error(const Token* t){
    fprintf(stderr, "invalid character %s\n", t->input);

    exit(1);
}
void error_s(const char* msg){
    fprintf(stderr, "Error: %s\n", msg);

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

        if(isdigit(*p)){
            tokens[i].type = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);

            ++i;
            continue;
        }

        {
            tokens[i].type = *p;
            tokens[i].input = p;
            ++i;
            ++p;
            continue;
        }
    }
}


Node* new_node(int op, Node* lhs, Node* rhs){
    Node* node = malloc(sizeof(Node));
    node->type = op;
    node->lhs = lhs;
    node->rhs = rhs;

    return node;
}
Node* new_node_num(int val){
    Node* node = malloc(sizeof(Node));
    node->type = ND_NUM;
    node->lhs = NULL;
    node->rhs = NULL;
    node->val = val;

    return node;
}

int pos = 0;
Node* expr();
Node* mul();
Node* term();

Node* expr(){
    Node* res = mul();
    if(tokens[pos].type == TK_EOF){
        return res;
    }

    while(1){
        if(tokens[pos].type == '+'){
            pos++;
            res = new_node('+', res, mul());
        }
        else if(tokens[pos].type == '-'){
            pos++;
            res = new_node('-', res, mul());
        }
        else break;
    }

    return res;
}

Node* mul(){
    Node* res = term();
    if(tokens[pos].type == TK_EOF){
        return res;
    }

    while(1){
        if(tokens[pos].type == '*'){
            pos++;
            res = new_node('*', res, term());
        }
        else if(tokens[pos].type == '/'){
            pos++;
            res = new_node('/', res, term());
        }
        else break;
    }

    return res;
}

Node* term(){
    if(tokens[pos].type == TK_NUM){
        return new_node_num(tokens[pos++].val);
    }

    if(tokens[pos].type == '('){
        pos++;
        Node* node = expr();
        if(tokens[pos].type != ')'){
            fprintf(stderr, "Unmatched bracket\n");
            error(&tokens[pos]);
        }
        ++pos;
        return node;
    }

    error(&tokens[pos]);
}

void gen(Node* node){
    if(node->type == ND_NUM){
        printf("  push %d\n", node->val);
        return;
    }
    if(!node->lhs || !node->rhs){
        error_s("operand argument error");
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  pop rdi\n");
    printf("  pop rax\n");

    const char* op;
    if(node->type == '+'){
        printf("  %s rax, rdi\n", "add");
    }
    else if(node->type == '-'){
        printf("  %s rax, rdi\n", "sub");
    }
    else if(node->type == '*'){
        printf("  mul rdi\n");
    }
    else if(node->type == '/'){
        printf("  mov rdx, 0\n");
        printf("  div rdi\n");
    }
    else{
        error_s("syntax error");
    }

    printf("  push rax\n");
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
    Node* node = expr();
    gen(node);

    printf("  pop rax\n");
    printf("  ret\n");

    return 0;
}