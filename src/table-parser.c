#include "table-parser.h"


List init_table_list(Symbol_table *table){
    List ls_table;
    
    if(!(ls_table = (List)malloc(sizeof(Table)))){
        fprintf(stderr, "Error while allocating list of symbol table");
        return NULL; 
    }

    ls_table->table = table;
    ls_table->next = NULL;

    return ls_table;

}

int insert_table(List list, Symbol_table *table){

    List tmp;

    for(tmp = list; tmp->next; tmp = tmp->next){}
    tmp->next = init_table_list(table);

    if(!tmp->next){
        fprintf(stderr, "Error while insert the table\n");
        return 0;
    }

    return 1;
}





