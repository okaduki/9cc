#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/**
 * global variables
 */
Vector *code = NULL;

/**
 * static variables
 */
int pos = 0;
Vector *tokens = NULL;


/**
 * init function
 */
void alloc_workspaces(){
    code = new_vector();
    tokens = new_vector();
}

void add_token(int type, int val, char *input){
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->val = val;
    token->input = input;

    vec_push(tokens, token);
}

Token* get_token(int idx){
    return (Token*)tokens->data[idx];
}

/**
 * Tokenize functions
 */
void tokenize(char* p){
    while(1){
        while(isspace(*p)) ++p;
        if(*p == '\0'){
            add_token(TK_EOF, 0, NULL);
            return;
        }

        if(isdigit(*p)){
            add_token(TK_NUM, strtol(p, &p, 10), p);
            continue;
        }

        if('a' <= *p && *p <= 'z'){
            add_token(TK_IDENT, 0, p);
            ++p;
            continue;
        }

        if(p[0] == '=' && p[1] == '='){
            add_token(TK_EQ, 0, p);
            p += 2;
            continue;
        }
        if(p[0] == '!' && p[1] == '='){
            add_token(TK_NE, 0, p);
            p += 2;
            continue;
        }
        if(p[0] == '<' && p[1] == '='){
            add_token(TK_LE, 0, p);
            p += 2;
            continue;
        }
        if(p[0] == '>' && p[1] == '='){
            add_token(TK_GE, 0, p);
            p += 2;
            continue;
        }

        {
            add_token(*p, 0, p);
            ++p;
            continue;
        }
    }
}

void printTokens(){
    for(int i=0;i<100;++i){
        Token *token = get_token(i);
        if(token->type == TK_EOF) return;
        fprintf(stderr, "type = %c(%d)\n", token->type, token->type);
    }
}

/**
 * parse functions
 */
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


void program();
Node* assign();
Node* assign_();
Node* expr();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* term();

int consume(int ty){
    if(get_token(pos)->type != ty) return 0;
    ++pos;
    return 1;
}

void program(){
    Node* res = assign();
    vec_push(code, res);
    if(get_token(pos)->type == TK_EOF){
        return;
    }
    program();
}

Node* assign(){
    Node* res = expr();
    if(consume('=')){
        res = new_node('=', res, assign_());
    }

    if(consume(';')){
        return res;
    }

    fprintf(stderr, "; required.");
    error(get_token(pos));

    return NULL;
}

Node* assign_(){
    Node* res = expr();

    if(get_token(pos)->type != '='){
        return res;
    }

    ++pos;
    return new_node('=', res, assign_());
}

Node* expr(){
    return equality();
}

// compは左結合
// 四則演算より結合が弱く、代入より強い
Node* equality(){
    Node* res = relational();
    if(consume(TK_EOF)){
        return res;
    }

    while(1){
        if(consume(TK_EQ)){
            res = new_node(ND_EQ, res, relational());
        }
        else if(consume(TK_NE)){
            res = new_node(ND_NE, res, relational());
        }
        else break;
    }

    return res;
}

Node* relational(){
    Node* res = add();
    if(consume(TK_EOF)){
        return res;
    }

    while(1){
        if(consume(TK_LE)){
            res = new_node(ND_LE, res, add());
        }
        else if(consume(TK_GE)){
            res = new_node(ND_LE, add(), res);
        }
        else if(consume('<')){
            res = new_node(ND_LT, res, add());
        }
        else if(consume('>')){
            res = new_node(ND_LT, add(), res);
        }
        else break;
    }

    return res;
}

Node* add(){
    Node* res = mul();
    if(consume(TK_EOF)){
        return res;
    }

    while(1){
        if(consume('+')){
            res = new_node('+', res, mul());
        }
        else if(consume('-')){
            res = new_node('-', res, mul());
        }
        else break;
    }

    return res;
}

Node* mul(){
    Node* res = unary();
    if(consume(TK_EOF)){
        return res;
    }

    while(1){
        if(consume('*')){
            res = new_node('*', res, unary());
        }
        else if(consume('/')){
            res = new_node('/', res, unary());
        }
        else break;
    }

    return res;
}

Node* unary(){
    if(consume('+')){
        return term();
    }
    else if(consume('-')){
        Node* res = term();
        return new_node('-', new_node_num(0), res);
    }

    return term();
}

Node* term(){
    if(get_token(pos)->type == TK_NUM){
        return new_node_num(get_token(pos++)->val);
    }

    if(get_token(pos)->type == TK_IDENT){
        return new_node_ident(get_token(pos++)->input[0]);
    }

    if(consume('(')){
        Node* node = expr();
        if(!consume(')')){
            fprintf(stderr, "Unmatched bracket\n");
            error(get_token(pos));
        }
        return node;
    }

    error(get_token(pos));
    return NULL;
}

