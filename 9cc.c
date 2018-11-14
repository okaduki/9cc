#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

enum {
    TK_NUM = 256,
    TK_IDENT,
    TK_EOF,
};

typedef struct {
    int type;
    int val;
    char* input;
} Token;

enum {
    ND_NUM = 256,
    ND_IDENT,
};

typedef struct Node {
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int val;
    char name;
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

        if('a' <= *p && *p <= 'z'){
            tokens[i].type = TK_IDENT;
            tokens[i].input = p;
            ++i;
            ++p;
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
Node* new_node_ident(char name){
    Node* node = malloc(sizeof(Node));
    node->type = ND_IDENT;
    node->lhs = NULL;
    node->rhs = NULL;
    node->name = name;

    return node;
}

int pos = 0;
void program();
Node* assign();
Node* assign_();
Node* expr();
Node* mul();
Node* term();

int npos = 0;
Node* code[100];
void program(){
    Node* res = assign();
    code[npos++] = res;
    if(tokens[pos].type == TK_EOF){
        return;
    }
    program();
}

Node* assign(){
    Node* res = expr();
    if(tokens[pos].type == '='){
        ++pos;
        res = new_node('=', res, assign_());
    }

    if(tokens[pos].type != ';'){
        fprintf(stderr, "; required.");
        error(&tokens[pos]);
    }
    ++pos;

    return res;
}

Node* assign_(){
    Node* res = expr();

    if(tokens[pos].type != '='){
        return res;
    }

    ++pos;
    return new_node('=', res, assign_());
}

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

    if(tokens[pos].type == TK_IDENT){
        return new_node_ident(tokens[pos++].input[0]);
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

void gen_lval(Node* node){
    if(node->type != ND_IDENT){
        error_s("rvalue cannot be assigned.");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", (node->name - 'a' + 1) * 8);
    printf("  push rax\n");
}

void gen(Node* node){
    if(node->type == ND_NUM){
        printf("  push %d\n", node->val);
        return;
    }
    if(node->type == ND_IDENT){
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    }
    
    if(node->type == '='){
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  pop rdi\n");
        printf("  pop rax\n");
        printf("  mov [rax], rdi\n");
        printf("  push rdi\n");
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