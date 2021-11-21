%{
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
void yyerror(char *s);
int yylex(void);
extern int lineno;
extern int line_count;
extern int linecharno;
struct Node* rootProg;

%}
%code requires {
#include "tree.h"
Node* root;
}
%expect 1
%union{
  Node *node;
}

%token DIVSTAR ADDSUB IDENT ORDER TYPE EQ OR AND NUM CHARACTER RETURN WHILE IF ELSE FOR VOID SWITCH DEFAULT CASE BREAK

%type <node> DeclVars Prog DeclFoncts TYPE Declarateurs DeclFonct EnTeteFonct Corps Parametres ListTypVar SuiteInstr Instr LValue Exp TB FB M E T F Arguments ListExp



%%
Prog:  DeclVars DeclFoncts              {$$ = makeNode(Prog); addChild($$, $1); addChild($$, $2); /*printTree($$); deleteTree($$);*/ rootProg = $$;}
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
Corps: '{' DeclVars SuiteInstr '}'      {$$ = makeNode(Body); addChild($$, $2); addSibling($2, $3);}
    ;           
SuiteInstr:                             
       SuiteInstr Instr                 {$$ = $1; addChild($$, $2);}
    |  %empty                           {$$ = makeNode(SuiteInstr);}
    ;
Instr:
       LValue '=' Exp ';'                   {$$ = makeNode(Assign); addChild($$, $1); addChild($$, $3);}
    |  IF '(' Exp ')' Instr                 {$$ = makeNode(If); addChild($$, $3); addChild($$, $5);}
    |  IF '(' Exp ')' Instr ELSE Instr      {$$ = makeNode(If); addChild($$, $3); Node *else_n = makeNode(Else); addSibling($$, else_n); addChild(else_n, $5);}
    |  SWITCH '(' IDENT ')' '{' BeginSwitchExpr '}'
    |  WHILE '(' Exp ')' Instr              {$$ = makeNode(While); addChild($$, $3); addChild($$, $5);}
    |  IDENT '(' Arguments  ')' ';'         {$$ = makeNode(types); addChild($$, $3);}
    |  RETURN Exp ';'                       {$$ = makeNode(Return); addChild($$, $2);}
    |  RETURN ';'                           {$$ = makeNode(Return);}
    |  '{' SuiteInstr '}'                   {$$ = $2;}
    |  ';'                                  {$$ = makeNode(EmptyInstr);}
    ;
Exp :  Exp OR TB                            {$$ = makeNode(Or); addChild($$, $1); addChild($$, $3);}
    |  TB                                   {$$ = $1;}
    ;
TB  :  TB AND FB                            {$$ = makeNode(And); addChild($$, $1); addChild($$, $3);}
    |  FB                                   {$$ = $1;}
    ;
FB  :  FB EQ M                              {$$ = makeNode(Eq); addChild($$, $1); addChild($$, $3);}
    |  M                                    {$$ = $1;}
    ;
M   :  M ORDER E                            {$$ = makeNode(Order); addChild($$, $1); addChild($$, $3);}
    |  E                                    {$$ = $1;}
    ;
E   :  E ADDSUB T                           {$$ = makeNode(Addsub); addChild($$, $1); addChild($$, $3);}
    |  T                                    {$$ = $1;}
    ;    
T   :  T DIVSTAR F                          {$$ = makeNode(divstar); addChild($$, $1); addChild($$, $3);}
    |  F                                    {$$ = $1;}
    ;
F   :  ADDSUB F                             {$$ = makeNode(Addsub); addChild($$, $2);}
    |  '!' F                                {$$ = makeNode(Neg);}
    |  '(' Exp ')'                          {$$ = $2;}
    |  NUM                                  {$$ = makeNode(Int);}
    |  CHARACTER                            {$$ = makeNode(Character);}
    |  LValue                               {$$ = $1;}
    |  IDENT '(' Arguments  ')'             {$$ = makeNode(FunctionCall); addChild($$, $3);}
    ;
LValue:
       IDENT                {$$ = makeNode(LValue);}
    ;

BeginSwitchExpr:
    BeginSwitchExpr Instr SwitchExpr
    | SwitchExpr
    ;
SwitchExpr: 
        CASE CHARACTER ':' EndSwitchExpr
    |   CASE NUM ':' EndSwitchExpr
    |   DEFAULT ':' EndSwitchExpr
    |   %empty
    ;
EndSwitchExpr:
        SuiteInstr BREAK ';' SwitchExpr
    ;
Arguments:
       ListExp              {$$ = makeNode(Arguments); addChild($$, $1);}
    |  %empty               {$$ = makeNode(Void);}
    ;
ListExp:
       ListExp ',' Exp      {$$ = makeNode(ListExp); addSibling($$, $3);}      
    |  Exp                  {$$ = $1;}
    ;   
%%
void yyerror(char *s){
    fprintf (stderr, "%s near line %d at char : %d\n", s, line_count, linecharno);
}

void print_help(){

    printf("=================================\n");
    printf("===========HELP GUIDE============\n");
    printf("=================================\n");
    printf("Command Line: ./tpcas [OPTION]\nWith OPTION equal to -h/--help or -t/--tree\n");
    printf("OPTION 1 : -h/--help , Print usage of the program.\n");
    printf("OPTION 2 : -t/--tree , Print the abstract tree on the standard output\n");
    printf("(You can use both options)\n");
}

int main(int argc, char **argv){
    int option;
    int showTree = 0;
    int result;
    int opt = 0;
    int option_index = 0;
    static struct option long_option[] = {

        {"help", no_argument,0,'h'},
        {"tree", no_argument,0,'t'},
        {0,0,0,0}
    };
    while((opt = getopt_long(argc, argv,"t h", long_option, &option_index)) !=-1 ){

        switch(opt){

            case 'h' : print_help();
                       break;

            case 't' : showTree = 1;
                       break;

            default : break;
        }
    }
    printf("%d\n", opt);
    while((option = getopt(argc, argv, ":th")) != - 1){
        switch(option){
            case 't':
                showTree = 1;
                break;
            case 'h':
                print_help();
                break;
            case '?':
                printf("unknown option\n");
                break;
        }
    }
    result = yyparse();
    if(result){
        return result;
    }
    if(showTree){
        printTree(rootProg);
    }
    deleteTree(rootProg);
    return result;
}
