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
    N_OR,
    CALL,
    CMP,
    JLE,
    JGE,
    JL,
    JNE,
    LEAVE,
    COMMENT,
    AND_N
}NasmFunCall;

#define FMTINT "formatIntIn"
#define FMTCHAR "fmtChar"

/**
 * @brief 
 * 
 * @param root 
 * @param list 
 */
void buildNasmFile(Node *root, List list);

/**
 * @brief 
 * 
 * @param fname 
 */
void makeExecutable(char *fname);


#endif