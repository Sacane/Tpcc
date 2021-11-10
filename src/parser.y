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
%expect 1
%union{
  Node *node;
}

%token DIVSTAR ADDSUB IDENT ORDER TYPE EQ OR AND NUM CHARACTER RETURN WHILE IF ELSE FOR VOID

%type <node> DeclVars Prog DeclFoncts TYPE Declarateurs DeclFonct EnTeteFonct Corps Parametres ListTypVar SuiteInstr Instr LValue Exp Body



%%
Prog:  DeclVars DeclFoncts              {$$ = makeNode(Prog); addChild($$, $1); addChild($$, $2); printTree($$); deleteTree($$);}
    ;
DeclVars:
       DeclVars TYPE Declarateurs ';'   {$$ = $1; addChild($$, $3);}
    |  %empty                           {$$ = makeNode(DeclVars);}
    ;
Declarateurs:  
       Declarateurs ',' IDENT           {$$ = $1; addChild($$, makeNode(id));}
    |  IDENT                            {$$ = makeNode(types); addChild($$, makeNode(id));}
    ;
DeclFoncts:
       DeclFoncts DeclFonct             {$$ = $1; addChild($$, $2);}
    |  DeclFonct                        {$$ = makeNode(DeclFoncts); addChild($$, $1);}
    ;
DeclFonct:
       EnTeteFonct Corps                {$$ = makeNode(DeclFonct); addChild($$, $1); addChild($$, $2);}                        
    ;
EnTeteFonct:
       TYPE IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(types); addChild($$, type); addChild(type, makeNode(id)); addChild($$, $4);}
    |  VOID IDENT '(' Parametres ')'    {$$ = makeNode(EnTeteFonct); Node * type = makeNode(types); addChild($$, makeNode(Void)); addChild(type, makeNode(id)); addChild($$, $4);}
    ;
Parametres:
       VOID                             {$$ = makeNode(Parametres); addChild($$, makeNode(Void));}
    |  ListTypVar                       {$$ = makeNode(Parametres); addChild($$, $1);}
    ;
ListTypVar:
       ListTypVar ',' TYPE IDENT        {$$ = $1; Node *type = makeNode(types); addSibling($$, type); addChild(type, makeNode(id));}
    |  TYPE IDENT                       {$$ = makeNode(types); addChild($$, makeNode(id));}
    ;
Corps: '{' DeclVars SuiteInstr '}'      {$$ = $2; addSibling($$, $3);}
    ;           
SuiteInstr:                             
       SuiteInstr Instr                 {$$ = $1; addChild($$, makeNode(Instr));}
    |  %empty                           {$$ = makeNode(Body);}
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
    |  %empty
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
