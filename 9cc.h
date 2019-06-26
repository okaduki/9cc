#ifndef __9CC_H__
#define __9CC_H__

enum {
    TK_NUM = 256,
    TK_IDENT,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
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
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_LT,
};

typedef struct Node {
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int val;
    char name;
} Node;

void error(const Token* t);
void error_s(const char* msg);


extern Node* code[100];

void tokenize(char* p);
void program();


void gen(Node* node);

#endif
