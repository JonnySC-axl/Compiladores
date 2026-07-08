%{
#include "parser.tab.h"
#include <stdlib.h>
%}

%%
[ \t\r]		{}
[\n]			{return EOL; }
[0-9]+(\.[0-9]+)?	{yylval.val = atof(yytext); return NUM; }
"+"			{return PLUS; }
"-"			{return MINUS; }
"*"			{return MULT; }
"/"			{return DIV; }
"("			{return LPAREN; }
")"			{return RPAREN; }
.			{printf("Error Lexico en '%s'\n", yytext); }
%%
int yywrap() {return 1;}
