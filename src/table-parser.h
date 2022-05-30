#ifndef __TABLE_PARSER__H
#define __TABLE_PARSER__H


#include "symbols-table.h"


#include <unistd.h>
#include  <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define OPEN  0
#define CLOSE 1

int check_sem_err;
int check_warn;
int hasMain;

typedef struct list_table{
    
    Symbol_table *table;
    struct list_table *next;

}Table, *List;

/**
 * @brief 
 * 
 * @return List 
 */
List newSymbolTableList();

/**
 * @brief 
 * 
 * @param list 
 * @param symbolTableList 
 * @return int 
 */
int insertSymbolTableInList(List list, Symbol_table *symbolTableList);

/**
 * @brief 
 * 
 * @param lst 
 */
void printSymbolTableList(List lst);

/**
 * @brief 
 * 
 * @param root 
 * @return List 
 */
List buildSymbolTableListFromRoot(Node *root);

Node * hasLabel(Node *root, label_t label);


/**
 * @brief Get the Table In List By its Name object
 * 
 * @param nameTable 
 * @param symbolTableList 
 * @return Symbol_table* 
 */
Symbol_table *getTableInListByName(char *nameTable, List symbolTableList);


/**
 * @brief free the memory of the symbol table
 * 
 */
void free_table(Symbol_table *symbolTable);


#endif