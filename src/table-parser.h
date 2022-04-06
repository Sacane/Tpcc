#ifndef __TABLE_PARSER__H
#define __TABLE_PARSER__H

#include "table-parser.h"
#include "symbols-table.h"


#include <unistd.h>
#include  <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#define OPEN  0
#define CLOSE 1

typedef struct list_table{
    
    Symbol_table *table;
    struct list_table *next;

}Table, *List;


List init_table_list();

int insert_table(List list, Symbol_table *table);
void print_chained_list(List lst);
List build_list_table(Node *root);
int treat_simple_sub_in_main(Node *root);
int parse_sem_function_error(Node *node, List table);
Symbol_table *get_table_by_name(char *name_table, List table_list);
void free_table();


#endif