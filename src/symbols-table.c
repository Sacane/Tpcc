#include "symbols-table.h"



static unsigned int hash_key(char name[]){
    int i;
    unsigned int res = 0;
    for(i = 0; i < strlen(name) -1; i++){
        res+=(i+1)*name[i];
        
    } 
    return res % MAX_SIZE_TABLE;
}


Symbol_table *create_symbol_table(char *name_table){
    Symbol_table *table;
    int i;

    table = malloc(sizeof(Symbol_table));
    table->name_table = malloc(sizeof(char) * strlen(name_table));
    table->size_table = 0;

    for(i = 0; i < MAX_SIZE_TABLE; i++){
        table->symbols[i] = calloc_symbol();
    }

    table->next = NULL;
    table->firstSibling = NULL;

    return table;
    
}

static int is_identifier_key_in_table(Symbol_table *table, int symbol_key){
    
    return (!is_symbol_null(table->symbols[symbol_key]));
}



int insert_symbol_in_table(Symbol symbol, Symbol_table *table){
    int key = hash_key(symbol.symbol_name);

    if(is_identifier_key_in_table(table, key)){
        return 0;
    }
    else{
        table->symbols[key] = symbol;
    }
    return 1;
}