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
    fprintf(stderr, "name : %s |\n", s.symbol_name);

    fprintf(stderr, "Kind : ");
    switch(s.kind){
        case FUNCTION:
            fprintf(stderr, "function\n");
            break;
        case VARIABLE:
            fprintf(stderr, "variable\n");
            break;
        default:
            fprintf(stderr, "Unknown\n");
            break;
    }

    fprintf(stderr, "Type : ");
    switch(s.type){
        case CHAR_TYPE: 
            fprintf(stderr, "char \n");
            break;
        case INT_TYPE:
            fprintf(stderr, "integer \n");
            break;
        case VOID_TYPE:
            fprintf(stderr, "void \n");
            break;
        default:
            fprintf(stderr, "unknown type\n");
    }

    fprintf(stderr, "\n");
}

void free_symbol(Symbol *s){
    free(s->symbol_name);
}

Type str_to_tpcType(char* type){
    Type t;
    if(strcmp(type, "int") == 0){
        return INT_TYPE;
    }
    else if(strcmp(type, "char") == 0){
        return CHAR_TYPE;
    }
    else if(strcmp(type, "void") == 0){
        return VOID_TYPE;
    }
    else {
        return NONE_TYPE;
    }
}