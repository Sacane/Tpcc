#include "symbol.h"

Symbol create_symbol(char *name, Kind kind, Type type){
    Symbol symbol;

    symbol.symbol_name = (char*) malloc(sizeof(char) * strlen(name));
    if(!symbol.symbol_name){
        fprintf(stderr, "Error while allocating symbol name\n");
        exit(EXIT_FAILURE);
    }
    strcpy(symbol.symbol_name, name);
    symbol.kind = kind;
    symbol.type = type;

    return symbol;

}

Symbol calloc_symbol(){
    Symbol nullSymbol;

    nullSymbol.kind = NONE_KIND;
    nullSymbol.type = NONE_TYPE;
    nullSymbol.symbol_name = NULL;

    return nullSymbol;
} 

int is_symbol_null(Symbol symbol){
    return (symbol.symbol_name) ? 1 : 0;
}

void print_symbol(Symbol s){
    printf("name : %s  |", s.symbol_name);

    printf("Kind : ");
    switch(s.kind){
        case FUNCTION:
            fprintf(stderr, "function ");
            break;
        case VARIABLE:
            fprintf(stderr, "variable ");
            break;
        default:
            fprintf(stderr, "Unknown ");
            break;
    }

    printf("Type : ");
    switch(s.type){
        case CHAR_TYPE: 
            fprintf(stderr, "char ");
            break;
        case INT_TYPE:
            fprintf(stderr, "integer ");
            break;
        case VOID_TYPE:
            fprintf(stderr, "void ");
            break;
        default:
            fprintf(stderr, "unknown type");
    }

    fprintf(stderr, "\n");
}

void free_symbol(Symbol *s){
    free(s->symbol_name);
}