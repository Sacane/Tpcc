#include "symbols-table.h"


unsigned long hash(unsigned char *str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}



Symbol_table *create_symbol_table(char *name_table){
    Symbol_table *table;
    int i;

    table = malloc(sizeof(Symbol_table));
    table->name_table = malloc(sizeof(char) * strlen(name_table));
    strcpy(table->name_table, name_table);
    table->nb_symbol = 0;
    if(!(table->s = malloc(sizeof(Symbol) * INIT_TABLE_SIZ))){
        fprintf(stderr, "Failed to allocated the symbol table : %s\n", name_table);
        return NULL;
    }
    table->size = INIT_TABLE_SIZ;
    if(!(table->parameters = malloc(sizeof(Type) * INIT_PARAMETERS_SIZ))){
        fprintf(stderr, "Failed to allocate the parameters\n");
        return NULL;
    }
    table->nb_parameter = 0;

    for(i = 0; i < INIT_TABLE_SIZ; i++){
        table->s[i] = calloc_symbol();
    }


    return table;
    
}

static int is_identifier_key_in_table(Symbol_table *table, int symbol_key){
    
    return (is_symbol_null(table->s[symbol_key]));
}



int insert_symbol_in_table(Symbol symbol, Symbol_table *table){
    unsigned long hashKey = hash(symbol.symbol_name);
    int i;
    if(table->size <= hashKey){

        table->s = realloc(table->s, sizeof(Symbol) * (hashKey + table->size + 1000));

        for(i = table->size; i < hashKey; i++){
            table->s[i] = calloc_symbol();
        }

        table->size = hashKey+1;
    }

    if(is_identifier_key_in_table(table, hashKey)){
        
        return 0;
    }
    else{
        table->s[hashKey] = symbol;
        table->nb_symbol += 1;
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
    fprintf(stderr,"Tab Name : %s!!\n",tab->name_table);
    fprintf(stderr,"Size : %d\n",tab->nb_symbol);
    for(pos = 0; pos < tab->size;pos++){
        if(tab->s[pos].symbol_name == NULL) continue;;
        printf("%s\n", tab->s[pos].symbol_name);
        print_symbol(tab->s[pos]);
        
    }
}