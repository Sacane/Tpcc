#ifndef __ASM_H
#define __ASM_H

#include "nasmProvider.h"

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