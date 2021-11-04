%{

/*ProjetAS-Lexer_v2*/

int line_count=0;
int carac_count=0;
int lexem_leng=0;

%}

%option nounput
%option noinput
%option noyywrap

%x LONG_COM SHORT_COM                 

%%

"/*" {BEGIN LONG_COM;}

<LONG_COM>"*/" {BEGIN INITIAL;}

"//" {BEGIN SHORT_COM;}

<SHORT_COM>\n {BEGIN INITIAL;} 

[*%/] {lexem_leng+= yyleng; return DIVSTAR;}
[+-] {lexem_leng+= yyleng; return ADDSUB;}
[a-zA-Z_][0-9a-zA-Z_]* {lexem_leng+= yyleng; return IDENT;}
"=>"| ">"| "<" || " <=" {lexem_leng+= yyleng; return ORDER;}
"int" | "char" { lexem_leng+= yyleng; return TYPE;}
"==" | "!=" {lexem_leng+= yyleng; return EQ;}
"||" {lexem_leng+= yyleng; return OR;}
"&&" {lexem_leng+= yyleng; return AND;}
[0-9][0-9]* {lexem_leng+= yyleng; return NUM;}
[a-zA-Z] {lexem_leng+= yyleng; return CHARACTER;}

"while" {lexem_leng+= yyleng; return WHILE;}
"for" {lexem_leng+= yyleng; return FOR;}
"if" {lexem_leng+= yyleng; return IF;}
"else" {lexem_leng+= yyleng; return ELSE;}
"return" {lexem_leng+= yyleng; return RETURN;}

\n {line_count++; carac_count = 0;}
. {carac_count++;}

%%
int main(){
    yylex();
    printf("nombre de lignes  = %d nombre de caractères = %d",line_count,carac_count);
}