9%{
#include <stdio.h>
#include <stdlib.h>

void yyerror(const char *s);
int yylex(void);
%}
%union{
	double val;
}
%token <val> NUM
%token PLUS MINUS DIV MULT LPAREN RPAREN
%token EOL

%type <val> expr term factor

%%
calc:

    | calc linea
    ;
linea:
     expr EOL { printf("Resultado: %f \n", $1); }
     ;
expr:
    expr PLUS term	{ $$ = $1 + $3; }
  | expr MINUS term	{ $$ = $1 - $3; }
  | term		{ $$ = $1; }
  | MINUS term 		{ $$ = -$2; }
  ;
term: 
    term MULT factor	{ $$ = $1 * $3; }
  | term DIV factor	{ 
				if($3 == 0.0){
					yyerror("Error semantico: Division por cero");
					exit(1);
					}
					$$ = $1 / $3;
				}
 | factor 			{ $$ = $1; }
 ;
factor: 
      LPAREN expr RPAREN { $$ = $2; }
    | NUM		 { $$ = $1; }
    ;
%%

void yyerror(const char *s){
	fprintf(stderr, "Error: %s \n", s);
}
int main(void){
	printf("Ingresa la expresion a evaluar: \n");
	return yyparse();
}


//bison -d parser.y
//flex lexer.l
//gcc parser.tab.c lex.yy.c -o cal -lfl
//./cal