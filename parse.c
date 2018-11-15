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
            tokens[i].type = TK_NEQ;
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
Node* comp();
Node* expr();
Node* mul();
Node* term();

void program(){
    Node* res = assign();
    code[npos++] = res;
    if(tokens[pos].type == TK_EOF){
        return;
    }
    program();
}

Node* assign(){
    Node* res = comp();
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
    Node* res = comp();

    if(tokens[pos].type != '='){
        return res;
    }

    ++pos;
    return new_node('=', res, assign_());
}

// compは左結合
// 四則演算より結合が弱く、代入より強い
Node* comp(){
    Node* res = expr();
    if(tokens[pos].type == TK_EOF){
        return res;
    }

    while(1){
        if(tokens[pos].type == TK_EQ){
            pos++;
            res = new_node(ND_EQ, res, expr());
        }
        else if(tokens[pos].type == TK_NEQ){
            pos++;
            res = new_node(ND_NEQ, res, expr());
        }
        else break;
    }

    return res;
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
        Node* node = comp();
        if(tokens[pos].type != ')'){
            fprintf(stderr, "Unmatched bracket\n");
            error(&tokens[pos]);
        }
        ++pos;
        return node;
    }

    error(&tokens[pos]);
    return NULL;
}

