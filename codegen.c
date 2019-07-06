#include <stdio.h>
#include "9cc.h"

static int label_counter = 0;

void gen_lval(Node* node){
    if(node->type != ND_LVAR){
        error_s("rvalue cannot be assigned.");
    }

    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->offset);
    printf("  push rax\n");
}

void gen(Node* node){
    if(!node){
        printf("  push rax\n");
        return;
    }

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

    if(node->type == ND_FUNC){
        for(int i=0;i<node->block->len;++i){
            gen((Node*)node->block->data[i]);
        }
        if(node->block->len >= 6) printf("  pop r9\n");
        if(node->block->len >= 5) printf("  pop r8\n");
        if(node->block->len >= 4) printf("  pop rcx\n");
        if(node->block->len >= 3) printf("  pop rdx\n");
        if(node->block->len >= 2) printf("  pop rsi\n");
        if(node->block->len >= 1) printf("  pop rdi\n");

        // https://stackoverflow.com/questions/9592345/x86-64-align-stack-and-recover-without-saving-registers
        printf("  push rsp\n");
        printf("  push [rsp]\n");
        printf("  and rsp, -0x10\n");
        printf("  call %s\n", node->name);
        printf("  mov rsp, [rsp+8]\n");
        return;
    }

    if(node->type == ND_BLOCK){
        for(int i=0;i<node->block->len; ++i){
            gen((Node*)node->block->data[i]);
            printf("  pop rax \n");
        }
        printf("  push rax \n");
        return;
    }

    if(node->type == ND_IF_COND){
        if(node->rhs->type != ND_IF_STM){
            error_s("no statements in if");
            return;
        }
        gen(node->lhs);

        int fi_label = label_counter++;
        if(node->rhs->rhs){ // exist else-statement
            int false_label = label_counter++;

            printf("  pop rax \n");
            printf("  cmp rax, 0 \n");
            printf("  je .L%d \n", false_label);

            // true
            gen(node->rhs->lhs);
            printf("  jmp .L%d \n", fi_label);
            // false
            printf(".L%d: \n", false_label);
            gen(node->rhs->rhs);
            // end
            printf(".L%d: \n", fi_label);
        }
        else{
            printf("  pop rax \n");
            printf("  cmp rax, 0 \n");
            printf("  je .L%d \n", fi_label);
            gen(node->rhs->lhs);
            printf("  pop rax \n");

            printf(".L%d: \n", fi_label);
            printf("  push rax \n");
        }
        return;
    }
    if(node->type == ND_WHILE){
        int begin_label = label_counter++;
        int end_label = label_counter++;

        printf(".L%d: \n", begin_label);
        // cond
        gen(node->lhs);
        printf("  pop rax \n");
        printf("  cmp rax, 0 \n");
        printf("  je .L%d \n", end_label);
        // stmt
        gen(node->rhs);
        printf("  pop rax \n");
        printf("  jmp .L%d \n", begin_label);
        // end
        printf(".L%d: \n", end_label);
        printf("  push rax \n");

        return;
    }
    if(node->type == ND_FOR){
        if(!node->lhs || node->lhs->type != ND_FOR
         || !node->rhs || node->rhs->type != ND_FOR){
            error_s("invalid syntax in for-expression");
            return;
        }

        Node* init = node->lhs->lhs;
        Node* cond = node->lhs->rhs;
        Node* incr = node->rhs->lhs;
        Node* stm  = node->rhs->rhs;

        int begin_label = label_counter++;
        int end_label = label_counter++;

        gen(init);
        printf("  pop rax \n");
        printf(".L%d: \n", begin_label);
        // cond
        gen(cond);
        printf("  pop rax \n");
        printf("  cmp rax, 0 \n");
        printf("  je .L%d \n", end_label);
        // stmt
        gen(stm);
        printf("  pop rax \n");
        // incr
        gen(incr);
        printf("  pop rax \n");
        printf("  jmp .L%d \n", begin_label);
        // end
        printf(".L%d: \n", end_label);
        printf("  push rax \n");

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