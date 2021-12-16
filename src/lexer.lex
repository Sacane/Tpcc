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
%x LONG_COMMENT SHORT_COMMENT
%option nounput
%option noinput
%option noyywrap
%%
"/*" {BEGIN LONG_COMMENT;}
"//"                {BEGIN SHORT_COMMENT;}
<LONG_COMMENT>"*/"        { BEGIN INITIAL; linecharno += yyleng; }
<LONG_COMMENT,SHORT_COMMENT>.        {linecharno += yyleng;}
<LONG_COMMENT>\n        {lineno++;linecharno=0;}
<SHORT_COMMENT>\n        {BEGIN INITIAL; lineno++;linecharno=0;}
'='|"=="|"!="	   {linecharno = linecharno + yyleng; return EQ;}
"<"|">"|"<="|">="	{linecharno = linecharno + yyleng; return ORDER;}
"+"|"-" 	        {linecharno = linecharno + yyleng; return ADDSUB;}
"/"|"*"|"%" 	    {linecharno = linecharno + yyleng; return DIVSTAR;}
void        {linecharno = linecharno + yyleng; return VOID;} 
if 		    {linecharno = linecharno + yyleng; return IF;}
else        {linecharno = linecharno + yyleng; return ELSE;}
while       {linecharno = linecharno + yyleng; return WHILE;}
return      {linecharno = linecharno + yyleng; return RETURN;}
switch      {linecharno = linecharno + yyleng; return SWITCH;}
case        {linecharno = linecharno + yyleng; return CASE;}
default     {linecharno = linecharno + yyleng; return DEFAULT;}
break       {linecharno = linecharno + yyleng; return BREAK;}
[0-9]+  	{linecharno = linecharno + yyleng; return NUM;}
"||"        {linecharno = linecharno + yyleng; return OR;}
"&&"        {linecharno = linecharno + yyleng; return AND;}
\'.\'|\'[ ]*\'|\'\\t\'	    {linecharno = linecharno + yyleng; return CHARACTER;}
" "|\t     {linecharno = linecharno + yyleng;}
int|char   {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return TYPE;}
[a-zA-Z_][a-zA-Z0-9_]*            {strncpy(yylval.ident, yytext, 64); linecharno = linecharno + yyleng; return IDENT;}
^.*[\n] {current_line = (char*)malloc(sizeof(char)*1024); strcpy(current_line, yytext); REJECT;}
. 	{linecharno = linecharno + yyleng; return yytext[0];}
\n {line_count++; linecharno = 0;}
%%