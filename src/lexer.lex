%{
#include "parser.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int lineno;
int linecharno = 0;
char* current_line = NULL;
%}
%x LONG_COM SHORT_COM
%option nounput
%option noinput
%option noyywrap


%%

"//"                {BEGIN SHORT_COM;}
<SHORT_COM>\n       {BEGIN INITIAL;} 
"/*"                BEGIN LONG_COM;
<LONG_COM>"*/"      BEGIN INITIAL;
'='|"=="|"!="	;   {return EQ;}
"<"|">"|"<="|">="	{return ORDER;}
"+"|"-" 	        {return ADDSUB;}
"/"|"*"|"%" 	    {return DIVSTAR;}
void        {return VOID;}
if 		    {return IF;}
else        {return ELSE;}
while       {return WHILE;}
return      {return RETURN;}
[0-9]+  	{return NUM;}
"||"        {return OR;}
"&&"        {return AND;}
\'.\' 	    {return CHARACTER;}
" "|\t     {linecharno = linecharno + yyleng;}
int|char   {return TYPE;}
[a-zA-Z_][a-zA-Z0-9_]*            {return IDENT;}
. ;	return yytext[0];


%%