#ifndef __SYMBOLS_TABLE
#define __SYMBOLS_TABLE

#include "symbol.h"
#include "tree.h"
#define MAX_SIZE_TABLE 255
#define INIT_NAME_TABLE_SIZE 20
#define INIT_TABLE_SIZ 10000
#define INIT_PARAMETERS_SIZ 10

typedef struct table{

    char *name_table;
    

    unsigned long size;
    Symbol *s;
    int nb_symbol;

    Type *parameters;   
    int nb_parameter;   //Its a functions's symbol table

}Symbol_table;




Symbol_table *create_symbol_table(char *name_table);

void get_symbol_by_name(Symbol_table *table, char *name_table);

int insert_symbol_in_table(Symbol symbol, Symbol_table *table);

Symbol_table *create_global_variable_table(Node *tree);

void print_symbol_table(Symbol_table *tab);

#endif