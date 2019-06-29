#ifndef __9CC_H__
#define __9CC_H__

/*

program    = stmt*
stmt    =
        | ";"
        | expr ";"
        | "if" "(" expr ")" stmt ("else" stmt)?
        | "while" "(" expr ")" stmt
        | "for" "(" expr? ";" expr? ";" expr? ")" stmt
        | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = ("+" | "-")? term
term       = num | ident | "(" expr ")"

 */
enum {
    TK_NUM = 256,
    TK_IDENT,
    TK_EQ,
    TK_NE,
    TK_LE,
    TK_GE,
    TK_RETURN,
    TK_IF,
    TK_ELSE,
    TK_WHILE,
    TK_FOR,
    TK_EOF,
};

typedef struct {
    int type;
    int val;
    char* input;
    int len;
} Token;

typedef struct LVar {
    struct LVar *next;
    char *name;
    int len;
    int offset;
} LVar;

enum {
    ND_NUM = 256,
    ND_EQ,
    ND_NE,
    ND_LE,
    ND_LT,
    ND_ASSIGN,
    ND_LVAR,
    ND_RETURN,
    ND_IF_COND,
    ND_IF_STM,
    ND_WHILE,
    ND_FOR,
    ND_EMPTY,
};

typedef struct Node {
    int type;
    struct Node* lhs;
    struct Node* rhs;
    int val;
    int offset;
} Node;

typedef struct {
    void **data;
    int capacity;
    int len;
} Vector;

Vector* new_vector();
void vec_push(Vector* vec, void *data);

void error(const Token* t);
void error_s(const char* msg);

//extern LVar* locals;
extern Vector* code;

void alloc_workspaces();
void tokenize(char* p);
void program();

LVar *find_lvar(Token *tok);

void gen(Node* node);

#endif
