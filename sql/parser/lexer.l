%{
#include <stdio.h>
#include "parser.h"

#define TOKEN(name) { return SQL_##name; }
%}

%option header-file="lexer.h"
%option outfile="lexer.cc"

%option noyywrap
%option case-insensitive
%option nounput

%option reentrant

%s COMMENT

%%

--				BEGIN(COMMENT);
<COMMENT>[^\n]*	/* skipping comment content until a end of line is read */;
<COMMENT>\n 	BEGIN(INITIAL);

[ \n\t]+	;

CHAR		TOKEN(CHAR)
CREATE		TOKEN(CREATE)
COLUMN		TOKEN(COLUMN)
COLUMNS		TOKEN(COLUMNS)

DROP		TOKEN(DROP)
DELETE		TOKEN(DELETE)
DOUBLE		TOKEN(DOUBLE)

FROM		TOKEN(FROM)
FALSE		TOKEN(FALSE)
FLOAT		TOKEN(FLOAT)

INT			TOKEN(INT)
INTO		TOKEN(INTO)
INDEX		TOKEN(INDEX)
INSERT		TOKEN(INSERT)
INTEGER		TOKEN(INTEGER)

LONG		TOKEN(LONG)

SET			TOKEN(SET)
SHOW		TOKEN(SHOW)
SELECT		TOKEN(SELECT)
SCHEMA		TOKEN(SCHEMA)
SCHEMAS		TOKEN(SCHEMAS)

TRUE		TOKEN(TRUE)
TABLE		TOKEN(TABLE)
TABLES		TOKEN(TABLES)
TRUNCATE	TOKEN(TRUNCATE)

UPDATE		TOKEN(UPDATE)
VALUES		TOKEN(VALUES)
VARCHAR		TOKEN(VARCHAR)
WHERE		TOKEN(WHERE)

"=="		TOKEN(EQUALS)             
"!=" 		TOKEN(NOTEQUALS)
"<>" 		TOKEN(NOTEQUALS)
"<="		TOKEN(LESSEQ)
">="		TOKEN(GREATEREQ)
"||"		TOKEN(CONCAT)

[-+*/(){},.;<>=^%:?[\]|]	{ return yytext[0]; }

-?[0-9]+"."[0-9]* |
"."[0-9]*	{
	yylval->fval = atof(yytext);
	return SQL_FLOATVAL;
}

-?[0-9]+	{
	yylval->ival = atol(yytext);
	return SQL_INTVAL;
}

\"[^\"\n]+\" {
	yylval->sval = substr(yytext, 1, strlen(yytext)-1);
	return SQL_IDENTIFIER;
}

[A-Za-z][A-Za-z0-9_]* {
	yylval->sval = strdup(yytext);
	return SQL_IDENTIFIER;
}

'[^'\n]*' {
	yylval->sval = yylval->sval = substr(yytext, 1, strlen(yytext)-1);
	return SQL_STRING;
}

. { fprintf(stderr, "[SQL-Lexer-Error] Unknown Character: %c\n", yytext[0]); return 0; }

%%

int yyerror(const char *msg) {
    fprintf(stderr, "[SQL-Lexer-Error] %s\n",msg); return 0;
}