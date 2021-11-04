%{

/*ProjetAS-Lexer_v2*/

int line_count=0;
int lexem_leng=0;
#include "parser.tab.h"
%}

%option nounput
%option noinput
%option noyywrap

%x LONG_COM SHORT_COM                 

%%
"//" {BEGIN SHORT_COM;}
<SHORT_COM>\n {BEGIN INITIAL;} 
"/*" {BEGIN LONG_COM;}
<LONG_COM>"*/" {BEGIN INITIAL;}
[0-9][0-9]* {return NUM;}
[a-zA-Z] {return CHARACTER;}
[a-zA-Z_][0-9a-zA-Z_]* {return IDENT;}
[*%/] {return DIVSTAR;}
[+-] {return ADDSUB;}
"=>"|">"|"<"|"<=" {return ORDER;}
"int"|"char" {return TYPE;}
"==" | "!=" {return EQ;}
"||" {return OR;}
"&&" {return AND;}
"while" {return WHILE;}
"for" {return FOR;}
"if" {return IF;}
"else" {return ELSE;}
"return" {return RETURN;}



\n {line_count++;}

%%
