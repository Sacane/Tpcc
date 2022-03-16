#ifndef __TABLE_PARSER__H
#define __TABLE_PARSER__H

#include "table-parser.h"
#include "symbols-table.h"


typedef struct list_table{
    
    Symbol_table *table;
    struct list_table *next;

}Table, *List;


List init_table_list();



void free_table();


#endif