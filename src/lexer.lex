%{
#include "parser.tab.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
int lineno;
int linecharno = 0;
int line_count = 1;
char* current_line = NULL;
%}
%x LONG_COM SHORT_COM
%option nounput
%option noinput
%option noyywrap
%%
"//"                {BEGIN SHORT_COM;}
<SHORT_COM>\n       {BEGIN INITIAL; linecharno = 0;} 
"/*"                {BEGIN LONG_COM;}
<LONG_COM>"*/"      {linecharno = linecharno + yyleng; BEGIN INITIAL;}
'='|"=="|"!="	   {linecharno = linecharno + yyleng; return EQ;}
"<"|">"|"<="|">="	{linecharno = linecharno + yyleng; return ORDER;}
"+"|"-" 	        {linecharno = linecharno + yyleng; return ADDSUB;}
"/"|"*"|"%" 	    {linecharno = linecharno + yyleng; return DIVSTAR;}
void        {linecharno = linecharno + yyleng; return VOID;}
if 		    {linecharno = linecharno + yyleng; return IF;}
else        {linecharno = linecharno + yyleng; return ELSE;}
while       {linecharno = linecharno + yyleng; return WHILE;}
return      {linecharno = linecharno + yyleng; return RETURN;}
[0-9]+  	{linecharno = linecharno + yyleng; return NUM;}
"||"        {linecharno = linecharno + yyleng; return OR;}
"&&"        {linecharno = linecharno + yyleng; return AND;}
\'.\' 	    {linecharno = linecharno + yyleng; return CHARACTER;}
" "|\t     {linecharno = linecharno + yyleng;}
int|char   {linecharno = linecharno + yyleng; return TYPE;}
[a-zA-Z_][a-zA-Z0-9_]*            {linecharno = linecharno + yyleng; return IDENT;}
. 	{linecharno = linecharno + yyleng; return yytext[0];}
\n {line_count++; linecharno = 0;}
%%