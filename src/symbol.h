#ifndef __SYMBOL
#define __SYMBOL

#include <stdio.h>
#include <stdlib.h>

typedef enum{

    FUNCTION, VARIABLE, NONE_KIND

}Kind;

typedef enum {
    CHAR_TYPE,
    INT_TYPE,
    VOID_TYPE,
    NONE_TYPE
}Type;


typedef struct symbol{


    char *symbol_name; /* name of the symbol  */
    Kind kind;         /* Kind of the symbol [function or variable] */
    Type type;        /* char, int and void (only for functions) */



}Symbol;

Symbol calloc_symbol();

int is_symbol_null(Symbol symbol);

Symbol create_symbol(char *name, Kind kind, Type type);

void free_symbol(Symbol *s);

#endif