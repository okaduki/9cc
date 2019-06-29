#include "9cc.h"
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>

/**
 * global variables
 */
Vector *code = NULL;
LVar *locals = NULL;

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

void add_token(int type, int val, char *input, int len){
    Token *token = malloc(sizeof(Token));
    token->type = type;
    token->val = val;
    token->input = input;
    token->len = len;

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
            add_token(TK_EOF, 0, NULL, 1);
            return;
        }

        if(isdigit(*p)){
            add_token(TK_NUM, strtol(p, &p, 10), p, 1);
            continue;
        }


        if(isalpha(*p)){
            char *q = p;
            int len = 0;
            while(p != NULL && (isalnum(*p) || *p == '_')){
                ++len;
                ++p;
            }

            if(len == 6 && strncmp(q, "return", len) == 0){
                add_token(TK_RETURN, 0, q, len);
                continue;
            }
            if(len == 2 && strncmp(q, "if", len) == 0){
                add_token(TK_IF, 0, q, len);
                continue;
            }
            if(len == 4 && strncmp(q, "else", len) == 0){
                add_token(TK_ELSE, 0, q, len);
                continue;
            }
            if(len == 5 && strncmp(q, "while", len) == 0){
                add_token(TK_WHILE, 0, q, len);
                continue;
            }

            add_token(TK_IDENT, 0, q, len);
            continue;
        }

        if(p[0] == '=' && p[1] == '='){
            add_token(TK_EQ, 0, p, 2);
            p += 2;
            continue;
        }
        if(p[0] == '!' && p[1] == '='){
            add_token(TK_NE, 0, p, 2);
            p += 2;
            continue;
        }
        if(p[0] == '<' && p[1] == '='){
            add_token(TK_LE, 0, p, 2);
            p += 2;
            continue;
        }
        if(p[0] == '>' && p[1] == '='){
            add_token(TK_GE, 0, p, 2);
            p += 2;
            continue;
        }

        {
            add_token(*p, 0, p, 1);
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
Node* new_node_ident(int offset){
    Node* node = malloc(sizeof(Node));
    node->type = ND_LVAR;
    node->lhs = NULL;
    node->rhs = NULL;
    node->offset = offset;

    return node;
}

LVar *find_lvar(Token *tok){
    for(LVar *crt = locals; crt != NULL; crt = crt->next){
        if(crt->len == tok->len && memcmp(crt->name, tok->input, crt->len) == 0){
            return crt;
        }
    }
    return NULL;
}


void program();
Node* stmt();
Node* expr();
Node* assign();
Node* assign_();
Node* equality();
Node* relational();
Node* add();
Node* mul();
Node* unary();
Node* term();

int consume(int ty){
    if(pos >= tokens->len || get_token(pos)->type != ty) return 0;
    ++pos;
    return 1;
}

void program(){
    Node* res = stmt();
    vec_push(code, res);
    if(get_token(pos)->type == TK_EOF){
        return;
    }
    program();
}

Node* stmt(){
    Node *res = NULL;
    if(consume(TK_RETURN)){
        res = expr();
        if(consume(';')){
            return new_node(ND_RETURN, res, NULL);
        }
    }
    else if(consume(TK_IF)){
        if(!consume('(')){
            fprintf(stderr, "( required.");
            error(get_token(pos));
        }
        Node* cond = expr();
        
        if(!consume(')')){
            fprintf(stderr, ") required.");
            error(get_token(pos));
        }

        Node* stm1 = stmt();
        Node* stm2 = NULL;

        if(consume(TK_ELSE)){
            stm2 = stmt();
        }

        return new_node(ND_IF_COND, cond, new_node(ND_IF_STM, stm1, stm2));
    }
    else if(consume(TK_WHILE)){
        if(!consume('(')){
            fprintf(stderr, "( required.");
            error(get_token(pos));
        }
        Node* cond = expr();
        
        if(!consume(')')){
            fprintf(stderr, ") required.");
            error(get_token(pos));
        }

        Node* stm = stmt();
        return new_node(ND_WHILE, cond, stm);
    }
    else{
        res = expr();

        if(consume(';')){
            return res;
        }
    }

    fprintf(stderr, "; required.");
    error(get_token(pos));

    return res;
}


Node* expr(){
    return assign();
}

Node* assign(){
    Node* res = equality();
    if(consume('=')){
        res = new_node(ND_ASSIGN, res, assign());
    }

    return res;
}

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
        Token *tok = get_token(pos++);
        LVar *var = find_lvar(tok);
        if(var == NULL){
            var = malloc(sizeof(LVar));
            var->next = locals;
            var->name = tok->input;
            var->len = tok->len;
            var->offset = (locals == NULL ? 0 : locals->offset) + 8;

            locals = var;
        }
        return new_node_ident(var->offset);
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

