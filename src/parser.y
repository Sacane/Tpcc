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
Node* root;
}

%union{
  Node *node;
}

%token DIVSTAR ADDSUB IDENT ORDER TYPE EQ OR AND NUM CHARACTER RETURN WHILE IF ELSE FOR VOID

%type <node> DeclVars Prog DeclFoncts TYPE Declarateurs id DeclFonct EnTeteFonct Corps



%%
Prog:  DeclVars DeclFoncts              {$$ = makeNode(Prog); addChild($$, $1); addChild($$, $2); printTree($$); deleteTree($$);}
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {addChild($$, $2); addChild($$, $3); }
    |  %empty                           {$$ = makeNode(DeclVars);}
    ;
Declarateurs:  
       Declarateurs ',' IDENT           {addChild($$, makeNode(id));}
    |  IDENT                            {$$ = makeNode(types); addChild($$, makeNode(id));}
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {addChild($$, $2);}
    |  DeclFonct                        {$$ = makeNode(DeclFoncts);}
    ;
DeclFonct:
       EnTeteFonct Corps                {$$ = makeNode(DeclFonct); addChild($$, makeNode(EnTeteFonct)); addChild($$, makeNode(Corps));}                        
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
