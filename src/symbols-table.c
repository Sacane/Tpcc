#include "symbols-table.h"


unsigned long hash(unsigned char *str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = hash + c; 

    return hash;
}




Symbol_table *create_symbol_table(char *name_table){
    Symbol_table *table;
    int i;
    
    table = malloc(sizeof(Symbol_table));
    table->name_table = malloc(sizeof(char) * strlen(name_table));
    strcpy(table->name_table, name_table);
    table->nb_symbol = 0;
    table->total_size = 0;
    if(!(table->s = malloc(sizeof(Symbol) * INIT_TABLE_SIZ))){
        fprintf(stderr, "Failed to allocated the symbol table : %s\n", name_table);
        return NULL;
    }

    table->size = INIT_TABLE_SIZ;
    table->nb_parameter = 0;

    for(i = 0; i < INIT_TABLE_SIZ; i++){
        table->s[i] = calloc_symbol();
    }


    return table;
    
}
int is_identifier_key_in_table(Symbol_table *table, unsigned long symbol_key){

    return (table->s[symbol_key].symbol_name != NULL);
}
Symbol get_symbol_by_name(Symbol_table *table, char *name_symbol){
    return table->s[hash(name_symbol)];
}

int is_symbol_in_table(Symbol_table *table, char *symbol_name){
    
    return table->size > hash(symbol_name) && table->s[hash(symbol_name)].symbol_name != NULL && !strcmp(table->s[hash(symbol_name)].symbol_name, symbol_name);
}





int insert_symbol_in_table(Symbol symbol, Symbol_table *table){
    unsigned long hashKey = hash(symbol.symbol_name);
    int i;
    if(table->size <= hashKey){
        long int old_size = table->size;
        table->size += hashKey;
        Symbol *s = (Symbol*)realloc(table->s, (sizeof(Symbol)) * table->size);
        
        if(!s){
            fprintf(stderr, "Error while realloc table->symbol | size : %ld\n", table->size);
            exit(EXIT_FAILURE);
        }
        table->s = s;
        for(i = old_size; i < hashKey; i++){

            table->s[i] = calloc_symbol();
            
        }
    }
    if(table->s[hash(symbol.symbol_name)].symbol_name && !strcmp(table->s[hash(symbol.symbol_name)].symbol_name, symbol.symbol_name)){
        return 0;
    }
    else{
        table->s[hashKey] = symbol;
        table->nb_symbol += 1;
    }
    //DEBUG("Insert symbol %s in table %s\n", table->s[hashKey].symbol_name, table->name_table);
    return 1;
}

int isPrimLabelNode(Node *n){
    switch(n->label){
        case Int:
        case Character:
            return 1;
        default:
            return 0;
    }
}

PrimType labelToPrim(label_t label){
    switch(label){
        case Int:
            return INT;
        case Character:
            return CHAR;
        default:
            return NONE;
    }
}

/* We suppose there is a var node */
Symbol_table *create_global_variable_table(Node *tree){
    Node* vars_as_node;
    Symbol_table *table = create_symbol_table("global_vars");
    Symbol symbol;
    PrimType type;
    int current_offset = 0, number_globals = 0;

    
    if (!(tree->firstChild)){
        return NULL;
    }

    vars_as_node = tree->firstChild; /* Node of DeclVars according to our tree */
    
    for (Node *child = vars_as_node->firstChild; child; child = child->nextSibling) {
        
        Kind kind = VARIABLE;
        
        type = (strcmp("int", child->u.ident) == 0) ? INT : CHAR;

        
        for(Node *grandChild = child->firstChild; grandChild; grandChild = grandChild->nextSibling){
            Symbol s = create_symbol(grandChild->u.ident, kind, type, current_offset, grandChild->lineno);
            insert_symbol_in_table(s, table);

            number_globals += 1;
            current_offset += 8;
        }
    }
    table->total_size = number_globals;
    
    
    return table;
}



void print_symbol_table(Symbol_table *tab){
    int pos;
    if(tab == NULL){
        printf("NULL \n");
        return;
    }

    fprintf(stderr,"Tab Name : %s\n",tab->name_table);
    fprintf(stderr,"nb symbol : %d\n",tab->nb_symbol);
    for(pos = 0; pos < tab->size;pos++){
        if(tab->s[pos].symbol_name == NULL) continue;;
        printf("%s\n", tab->s[pos].symbol_name);
        print_symbol(tab->s[pos]);
        
    }
}

