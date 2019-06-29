#include <stdio.h>
#include "9cc.h"


void gen_lval(Node* node){
    if(node->type != ND_LVAR){
        error_s("rvalue cannot be assigned.");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node* node){
    if(node->type == ND_NUM){
        printf("  push %d\n", node->val);
        return;
    }
    if(node->type == ND_LVAR){
        gen_lval(node);
        printf("  pop rax\n");
        printf("  mov rax, [rax]\n");
        printf("  push rax\n");
        return;
    }
    
    if(node->type == ND_RETURN){
        gen(node->lhs);
        printf("  pop rax \n");
        printf("  mov rsp, rbp \n");
        printf("  pop rbp \n");
        printf("  ret\n");
        return;
    }

    if(node->type == ND_ASSIGN){
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

    switch(node->type){
    case '+':
        printf("  %s rax, rdi\n", "add");
        break;
    case '-':
        printf("  %s rax, rdi\n", "sub");
        break;
    case '*':
        printf("  mul rdi\n");
        break;
    case '/':
        printf("  mov rdx, 0\n");
        printf("  div rdi\n");
        break;
    case ND_EQ:
        printf("  cmp rax, rdi\n");
        printf("  sete al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_NE:
        printf("  cmp rax, rdi\n");
        printf("  setne al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LE:
        printf("  cmp rax, rdi\n");
        printf("  setle al\n");
        printf("  movzb rax, al\n");
        break;
    case ND_LT:
        printf("  cmp rax, rdi\n");
        printf("  setl al\n");
        printf("  movzb rax, al\n");
        break;
    default:
        error_s("syntax error");
    }

    printf("  push rax\n");
}