#ifndef __9CC_H__
#define __9CC_H__

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

void error(const Token* t);
void error_s(const char* msg);


extern Node* code[100];

void tokenize(char* p);
void program();


void gen(Node* node);

#endif
