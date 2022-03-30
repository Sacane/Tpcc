#include "symbol.h"



Symbol create_symbol(char *name, Kind kind, PrimType type){
    Symbol symbol;

    symbol.symbol_name = (char*) malloc(sizeof(char) * strlen(name));
    if(!symbol.symbol_name){
        fprintf(stderr, "Error while allocating symbol name\n");
        exit(EXIT_FAILURE);
    }
    strcpy(symbol.symbol_name, name);
    symbol.kind = kind;
    symbol.u.p_type = type;

    return symbol;

}

Symbol create_func_sym(char *name_func, PrimType return_type, PrimType arg_types[], int n_args){
    Symbol func_sym;
    int i;
    func_sym.symbol_name = (char*) malloc(sizeof(char) * strlen(name_func));
    if(!func_sym.symbol_name){
        fprintf(stderr, "Error while allocating symbol name\n");
        exit(EXIT_FAILURE);
    }
    strcpy(func_sym.symbol_name, name_func);

    func_sym.kind = FUNCTION;
    func_sym.u.f_type.is_void = (return_type == NONE) ? 1 : 0;
    if(func_sym.u.f_type.is_void){
        return func_sym;
    }
    func_sym.u.f_type.return_type = return_type;
    for(i = 0; i < n_args; i++){
        func_sym.u.f_type.args_types[i] = arg_types[i];
        
    }
    
    
    return func_sym;
}

Symbol calloc_symbol(){
    Symbol nullSymbol;

    nullSymbol.kind = NONE_KIND;
    nullSymbol.symbol_name = NULL;

    return nullSymbol;
} 

int is_symbol_null(Symbol symbol){
    return (symbol.kind == NONE_KIND) ? 1 : 0;
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
        case PARAM:
            fprintf(stderr, "parametre\n");
            break;
        default:
            fprintf(stderr, "Unknown\n");
            break;
    }
}

void free_symbol(Symbol *s){
    free(s->symbol_name);
}



PrimType str_to_tpcType(char* type){

    if(strcmp(type, "int") == 0){
        return INT;
    }
    else if(strcmp(type, "char") == 0){
        return CHAR;
    }
    else {
        return NONE;
    }
}

char * string_from_type(PrimType t){
    switch (t){
        case INT:
            return "int";
            break;
        case CHAR:
            return "char";
            break;
        default:
            return "none";
            break;
    }
}