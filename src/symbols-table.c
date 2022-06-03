#include "symbols-table.h"

#define PUTINT 0
#define PUTCHAR 1
#define GETINT 2
#define GETCHAR 3

unsigned long hash(unsigned char *str)
{
    unsigned long hash = 0;
    int c;

    while (c = *str++)
        hash = hash + c; 

    return hash;
}

Symbol_table *newSymbolTable(char *name_table){
    Symbol_table *table;
    int i;
    if(!(table = malloc(sizeof(Symbol_table)))){
        raiseError(-1, "Failed to allocate the table\n");
        return NULL;
    }
    if(!(table->name_table = malloc(sizeof(char) * strlen(name_table)))){
        raiseError(-1, "Failed to allocate the table's name\n");
        return NULL;
    }
    if(!(table->s = malloc(sizeof(Symbol) * INIT_TABLE_SIZ))){
        DEBUG("Failed to allocated the symbol table : %s\n", name_table);
        return NULL;
    }
    strcpy(table->name_table, name_table);
    table->nb_symbol = 0;
    table->total_size = 0;
    table->size = INIT_TABLE_SIZ;
    table->nb_parameter = 0;
    table->u.number_globals = 0;
    for(i = 0; i < INIT_TABLE_SIZ; i++){
        table->s[i] = calloc_symbol();
    }
    return table;
}

Symbol getSymbolInTableByName(Symbol_table *table, char *symbolName){
    return table->s[hash(symbolName)];
}

int isSymbolInTable(Symbol_table *table, char *symbol_name){
    return table->size > hash(symbol_name) && 
    table->s[hash(symbol_name)].symbol_name != NULL && 
    strcmp(table->s[hash(symbol_name)].symbol_name, symbol_name) == 0;
}

static int realloc_table(Symbol_table *table, unsigned long hashkey){
    Symbol *s;
    int i;
    if(table->size <= hashkey){
        long int old_size = table->size;
        table->size += hashkey;
        if(!(s = (Symbol*)realloc(table->s, (sizeof(Symbol)) * table->size))){
            raiseError(-1, "Error while realloc table->symbol | size : %ld\n", table->size);
            exit(EXIT_FAILURE);
        }
        table->s = s;
        for(i = old_size; i < hashkey; i++){
            table->s[i] = calloc_symbol();
        }
    }
    return 1;
}

int insertSymbol(Symbol symbol, Symbol_table *table){
    Symbol *s;
    int i;
    unsigned long hashKey = hash(symbol.symbol_name);
    realloc_table(table, hashKey);
    if(table->s[hash(symbol.symbol_name)].symbol_name && !strcmp(table->s[hash(symbol.symbol_name)].symbol_name, symbol.symbol_name)){
        raiseError(symbol.lineno, "Symbol '%s' already declared as parameter or local variable\n", symbol.symbol_name);
        check_sem_err = 1;
        return 0;
    } else {
        table->s[hashKey] = symbol;
        table->nb_symbol += 1;
    }
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
Symbol_table *buildGlobalVariableSymbolTable(Node *root){
    Symbol symbol;
    PrimType type;
    Symbol_table *table = newSymbolTable(GLOBAL);
    int currentOffset = 0, nbGlobals = 0;
    if (!(root->firstChild)){
        return NULL;
    }
    Node *nodeVars = FIRSTCHILD(root); /* Node of DeclVars according to our tree */
    
    //Variable globaux
    for (Node *child = nodeVars->firstChild; child; child = child->nextSibling) {
        Kind kind = VARIABLE;
        type = (strcmp("int", child->u.ident) == 0) ? INT : CHAR;
        for(Node *grandChild = child->firstChild; grandChild; grandChild = grandChild->nextSibling){
            Symbol s = newSymbol(grandChild->u.ident, kind, type, currentOffset, grandChild->lineno);
            insertSymbol(s, table);
            nbGlobals += 1;
            currentOffset += 8;
        }
    }

    table->total_size = nbGlobals;
    return table;
}



void printSymbolTable(Symbol_table *tab){
    int pos;
    if(tab == NULL){
        printf("NULL \n");
        return;
    }

    fprintf(stderr,"TAB NAME : %s\n",tab->name_table);
    fprintf(stderr,"nb symbol : %d\n",tab->nb_symbol);
    for(pos = 0; pos < tab->size;pos++){
        if(tab->s[pos].symbol_name == NULL) continue;;
        print_symbol(tab->s[pos]);
        
    }
}

Symbol_table *buildPrimaryFunction(int funId){
    Symbol_table *funTable;
    char buf[10];
    Symbol s;
    switch(funId){
        case PUTINT:
            funTable = newSymbolTable("putint");
            sprintf(s.symbol_name, "putint");
            s.u.f_type.is_void = 1;
            s.u.f_type.nb_args = 1;
            s.u.f_type.args_types[0] = INT;
            break;
        case PUTCHAR:
            funTable = newSymbolTable("putchar");
            sprintf(s.symbol_name, "putchar");
            s.u.f_type.is_void = 1;
            s.u.f_type.nb_args = 1;
            s.u.f_type.args_types[0] = CHAR;
            break;
        case GETCHAR:
            funTable = newSymbolTable("getchar");
            s.u.f_type.is_void = 0;
            s.u.f_type.return_type = CHAR;
            sprintf(s.symbol_name, "getchar");
            break;
        case GETINT:
            funTable = newSymbolTable("getint");
            s.u.f_type.is_void = 0;
            s.u.f_type.return_type = INT;
            sprintf(s.symbol_name, "getint");
            break;
    }
    s.kind = FUNCTION;
    s.lineno = 0;
    s.offset = 0;
    s.u.f_type.nb_local = 0;
    insertSymbol(s, funTable);
    return funTable;
}