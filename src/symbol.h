#ifndef __SYMBOL
#define __SYMBOL

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_FUNC_NAME_SIZ 64
#define MAX_ARGUMENT_FUNC 15

typedef enum{

    FUNCTION, VARIABLE, VOID_KIND, NONE_KIND

}Kind;

typedef enum {

    CHAR,
    INT,

}PrimType;

typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    VOID_TYPE,
    NONE_TYPE
}Type;

typedef struct {

    char function_name[MAX_FUNC_NAME_SIZ];
    int is_void;
    PrimType args_types[MAX_ARGUMENT_FUNC];
    PrimType return_type;

}FunctionType;


typedef struct symbol{


    char *symbol_name; /* name of the symbol  */
    Kind kind;         /* Kind of the symbol [function, variable or parameter] */
    Type type;        /* char, int and 'void (only for functions)' */

    union {
        FunctionType f_type; /* Function  symbol */
        PrimType p_type;     /* Primitive symbol */
    }u;

}Symbol;

Symbol calloc_symbol();

int is_symbol_null(Symbol symbol);

Symbol create_symbol(char *name, Kind kind, Type type);

Type str_to_tpcType(char* type);

void print_symbol(Symbol s);

void free_symbol(Symbol *s);

#endif