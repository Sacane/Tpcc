#ifndef __SYMBOLS_TABLE
#define __SYMBOLS_TABLE

#include "symbol.h"
#define MAX_SIZE_TABLE 255
#define INIT_NAME_TABLE_SIZE 20

typedef struct table{

    char *name_table;

    int size_table; /* Size of the table */
    Symbol symbols[MAX_SIZE_TABLE]; /* hash table of the symbols */

    struct table *next;
    struct table *firstSibling;

}Symbol_table;


Symbol_table *create_symbol_table();

void get_symbol_by_name(Symbol_table *table, char *name_table);

int insert_symbol_in_table(Symbol symbol, Symbol_table *table);


#endif