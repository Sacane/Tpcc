#ifndef __ASM_H
#define __ASM_H

#include "tree.h"
#include "table-parser.h"


typedef enum {
    PUSH,
    POP,
    MOV,
    ADD,
    SUB,
    MUL,
    DIV,
    NOT,
    SYSCALL,
    JMP,
    JG,
    JE,
    RET,
    N_AND,
    N_OR
}NasmFunCall;


void build_asm(Node *root, List list);



#endif