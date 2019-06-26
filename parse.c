#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/**
 * global variables
 */
Node* code[100];

/**
 * static variables
 */
int pos = 0;
Token tokens[100];
int npos = 0;

/**
 * Tokenize functions
 */
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

        if(p[0] == '=' && p[1] == '='){
            tokens[i].type = TK_EQ;
            tokens[i].input = p;
            ++i;
            p += 2;
            continue;
        }
        if(p[0] == '!' && p[1] == '='){
            tokens[i].type = TK_NE;
            tokens[i].input = p;
            ++i;
            p += 2;
            continue;
        }
        if(p[0] == '<' && p[1] == '='){
            tokens[i].type = TK_LE;
            tokens[i].input = p;
            ++i;
            p += 2;
            continue;
        }
        if(p[0] == '>' && p[1] == '='){
            tokens[i].type = TK_GE;
            tokens[i].input = p;
            ++i;
            p += 2;
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

void printTokens(){
    for(int i=0;i<100;++i){
        if(tokens[i].type == TK_EOF) return;
        fprintf(stderr, "type = %c(%d)\n", tokens[i].type, tokens[i].type);
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
    if(tokens[pos].type != ty) return 0;
    ++pos;
    return 1;
}

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
    if(consume('=')){
        res = new_node('=', res, assign_());
    }

    if(consume(';')){
        return res;
    }

    fprintf(stderr, "; required.");
    error(&tokens[pos]);

    return NULL;
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
    if(tokens[pos].type == TK_NUM){
        return new_node_num(tokens[pos++].val);
    }

    if(tokens[pos].type == TK_IDENT){
        return new_node_ident(tokens[pos++].input[0]);
    }

    if(consume('(')){
        Node* node = expr();
        if(!consume(')')){
            fprintf(stderr, "Unmatched bracket\n");
            error(&tokens[pos]);
        }
        return node;
    }

    error(&tokens[pos]);
    return NULL;
}

