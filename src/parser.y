%{
#include <stdlib.h>
#include <stdio.h>
void yyerror(char *s);
int yylex(void);
extern int lineno;
extern int line_count;
extern int linecharno;

%}
%code requires {
#include "tree.h"
struct Node* root;
}

%token DIVSTAR ADDSUB IDENT ORDER TYPE EQ OR AND NUM CHARACTER RETURN WHILE IF ELSE FOR VOID

%%
Prog:  DeclVars DeclFoncts 
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';' 
    |
    ;
Declarateurs:
       Declarateurs ',' IDENT 
    |  IDENT 
    ;
DeclFoncts:
       DeclFoncts DeclFonct 
    |  DeclFonct 
    ;
DeclFonct:
       EnTeteFonct Corps 
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')' 
    |  VOID IDENT '(' Parametres ')' 
    ;
Parametres:
       VOID 
    |  ListTypVar 
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT 
    |  TYPE IDENT 
    ;
Corps: '{' DeclVars SuiteInstr '}' 
    ;
SuiteInstr:
       SuiteInstr Instr 
    |
    ;
Instr:
       LValue '=' Exp ';'
    |  IF '(' Exp ')' Instr 
    |  IF '(' Exp ')' Instr ELSE Instr
    |  WHILE '(' Exp ')' Instr
    |  IDENT '(' Arguments  ')' ';'
    |  RETURN Exp ';' 
    |  RETURN ';' 
    |  '{' SuiteInstr '}' 
    |  ';' 
    ;
Exp :  Exp OR TB 
    |  TB 
    ;
TB  :  TB AND FB 
    |  FB 
    ;
FB  :  FB EQ M
    |  M
    ;
M   :  M ORDER E 
    |  E 
    ;
E   :  E ADDSUB T 
    |  T 
    ;    
T   :  T DIVSTAR F 
    |  F 
    ;
F   :  ADDSUB F 
    |  '!' F 
    |  '(' Exp ')' 
    |  NUM 
    |  CHARACTER
    |  LValue
    |  IDENT '(' Arguments  ')' 
    ;
LValue:  
       IDENT 
    ;
Arguments:
       ListExp 
    |
    ;
ListExp:
       ListExp ',' Exp 
    |  Exp 
    ;
%%
void yyerror(char *s){
    fprintf (stderr, "%s near line %d at char : %d\n", s, line_count, linecharno);
}

int main(void){
    return yyparse();
}
