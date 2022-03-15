#include "symbols-table.h"


unsigned long hash(unsigned char *str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

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
    
    return (is_symbol_null(table->symbols[symbol_key]));
}



int insert_symbol_in_table(Symbol symbol, Symbol_table *table){
    int key = hash_key(symbol.symbol_name);
    printf("Key : %d\n", key);
    if(is_identifier_key_in_table(table, key)){
        return 0;
    }
    else{
        table->symbols[key] = symbol;
        table->size_table += 1;
    }
    return 1;
}


/* We suppose there is a var node */
Symbol_table *create_global_variable_table(Node *tree){
    Node* vars_as_node;
    Symbol_table *table = create_symbol_table("global_vars");
    Symbol symbol;
    Type type;

    if (!(tree->firstChild)){
        return NULL;
    }

    vars_as_node = tree->firstChild; /* Node of DeclVars according to our tree */
    
    for (Node *child = vars_as_node->firstChild; child != NULL; child = child->nextSibling) {
        printf("%s\n", stringFromLabel(child->label));
        Kind kind = VARIABLE;
        
        type = (strcmp("int", child->u.ident) == 0) ? INT_TYPE : CHAR_TYPE;



        for(Node *grandChild = child->firstChild; grandChild != NULL; grandChild = grandChild->nextSibling){
            Symbol s = create_symbol(grandChild->u.ident, kind, type);
            print_symbol(s);
            printf("%s\n", (insert_symbol_in_table(s, table) ? "Insert success\n" : "Insert failed\n"));
        }
    }


    return table;
}


void print_symbol_table(Symbol_table *tab){
    int pos;
    fprintf(stderr,"Tab Name : %s\n",tab->name_table);
    fprintf(stderr,"Size : %d\n",tab->size_table);
    for(pos = 0; pos < MAX_SIZE_TABLE;pos++){
        if(tab->symbols[pos].symbol_name == NULL) continue;;
        if(!(is_symbol_null(tab->symbols[pos]))) print_symbol(tab->symbols[pos]);
            
    }
}