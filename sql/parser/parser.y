%{
#include "parser.h"
#include "lexer.h"

#include <stdio.h>
#include <string.h>

int yyerror(YYLTYPE* llocp, parser_result* result, yyscan_t scanner, const char* msg) {
    result->set_valid(false);
    result->set_error(strdup(msg), llocp->first_line, llocp->first_column);
    return 0;
}
%}

%code requires {
#include "../statement/create_statement.h"
#include "../statement/delete_statement.h"
#include "../statement/drop_statement.h"
#include "../statement/insert_statement.h"
#include "../statement/select_statement.h"
#include "../statement/show_statement.h"
#include "../statement/update_statement.h"
#include "../parser_result.h"
#include "../../table/record.h"
#include "../../table/table.h"

#define YY_USER_ACTION \
		yylloc->first_line = yylloc->last_line; \
		yylloc->first_column = yylloc->last_column; \
		for(int i = 0; yytext[i] != '\0'; i++) { \
			yylloc->total_column++; \
			yylloc->string_length++; \
				if(yytext[i] == '\n') { \
						yylloc->last_line++; \
						yylloc->last_column = 0; \
				} \
				else { \
						yylloc->last_column++; \
				} \
		}
}

%output  "parser.cc"
%defines "parser.h"

%define api.pure full
%define parse.error verbose
%locations

%initial-action {
	@$.first_column = 0;
	@$.last_column = 0;
	@$.first_line = 0;
	@$.last_line = 0;
	@$.total_column = 0;
	@$.string_length = 0;
};

%lex-param   { yyscan_t scanner }
%parse-param { parser_result* result }
%parse-param { yyscan_t scanner }

%union {
    double    fval;
    int64_t   ival;
    char*     sval;
    bool      bval;

    statement*        stmt;
    expression*       expr;
    create_statement* create_stmt;
    insert_statement* insert_stmt;
    select_statement* select_stmt;
    update_statement* update_stmt;
    delete_statement* delete_stmt;
    drop_statement*   drop_stmt;
    show_statement*   show_stmt;

    table_name*        table_name;
	column_definition* column_t;
	column_type        column_type_t;
	update_clause*     update_t;

	std::vector<statement *>*         stmt_vec;
	std::vector<char*>*               str_vec;
	std::vector<column_definition *>* column_vec;
	std::vector<update_clause *>*     update_vec;
	std::vector<expression *>*        expr_vec;
}

%destructor { } <fval> <ival> <bval>
%destructor { free( ($$.table_name) ); free( ($$.schema_name) ); } <table_name>
%destructor { free( ($$) ); } <sval>
%destructor {
	if (($$) != nullptr) {
		for (auto ptr : *($$)) {
			delete ptr;
		}
	}
	delete ($$);
} <str_vec> <column_vec> <update_vec> <expr_vec> <stmt_vec>
%destructor { delete ($$); } <*>

%token <sval> IDENTIFIER STRING
%token <fval> FLOATVAL
%token <ival> INTVAL

%token COLUMNS CREATE CHAR
%token DROP DELETE DOUBLE
%token FLOAT FROM FALSE
%token INT INTO INDEX INSERT INTEGER 
%token LONG
%token SET SHOW SELECT
%token TABLE TABLES TRUE TRUNCATE
%token UPDATE VALUES VARCHAR WHERE

%type <stmt_vec>	    statement_list
%type <statement> 	    statement
%type <select_stmt>     select_stmt
%type <create_stmt>     create_stmt
%type <insert_stmt>     insert_stmt
%type <delete_stmt>     delete_stmt truncate_stmt
%type <update_stmt>     update_stmt
%type <drop_stmt>	    drop_stmt
%type <show_stmt>	    show_stmt
%type <table_name>      table_name
%type <bval> 		    opt_column_nullable
%type <expr> 		    expr operand scalar_expr
%type <expr>		    between_expr
%type <expr> 		    column_name literal int_literal num_literal string_literal bool_literal
%type <expr> 		    comp_expr opt_where
%type <expr> 		   	null_literal
%type <column_t>	    column_def
%type <column_type_t>   column_type
%type <update_t>	    update_clause
%type <expr_vec> 	    expr_list literal_list
%type <update_vec>	    update_clause_commalist
%type <column_vec>	    column_def_commalist
%type <str_vec>		    ident_commalist opt_column_list

%nonassoc	'=' EQUALS NOTEQUALS
%nonassoc	'<' '>' LESS GREATER LESSEQ GREATEREQ

%left		'(' ')'
%left		'.'
%%

input:
        statement_list opt_semicolon {
            for (statement* stmt : *$1) {
                result->add_statement(stmt);
            }
        }
    ;

statement_list:
        statement {
            $1->string_length = yylloc.string_length;
			yylloc.string_length = 0;
            $$ = new std::vector<statement *>();
            $$->push_back($1);
        }
    |   statement_list ';' statement {
            $3->string_length = yylloc.string_length;
			yylloc.string_length = 0;
            $1->push_back($3);
            $$ = $1;
    }
    ;

statement:
    	select_stmt { $$ = $1; }
	|	create_stmt { $$ = $1; }
	|	insert_stmt { $$ = $1; }
	|	delete_stmt { $$ = $1; }
	|	truncate_stmt { $$ = $1; }
	|	update_stmt { $$ = $1; }
	|	drop_stmt { $$ = $1; }
	;

show_stmt:
        SHOW TABLES {
            $$ = new show_statement(showtype::TABLES);
        }
    |   SHOW COLUMNS table_name {
            $$ = new show_statement(showtype::COLUMNS);
            $$->schema_name = $3.schema_name;
            $$->table_name = $3.table_name;
        }
    ;

create_stmt:
        CREATE TABLE table_name '(' column_def_commalist ')' {
            $$ = new create_statement(create_type::CREATE_TABLE);
            $$->schema_name = $3.schema_name;
            $$->table_name = $3.table_name;
            $$->columns = $5;
        }
    |   CREATE INDEX index_name ON table_name '(' column_def ')' {
            $$ = new create_statement(create_type::CREATE_INDEX);
            $$->index_name = $3;
            $$->schema_name = $5.schema_name;
            $$->table_name = $5.table_name;
            $$->index_column = $7;
        }
    ;

column_def_commalist:
        column_def { 
            $$ = new std::vector<column_definition *>(); 
            $$->push_back($1); 
        }
    |   column_def_commalist ',' column_def { 
            $1->push_back($3); 
            $$ = $1; 
        }
    ;

column_def:
        IDENTIFIER column_type opt_column_nullable {
            $$ = new column_definition($1, $2, $3);
        }
    ;

column_type:
        INT                    { $$ = new column_type(data_type::INT); }
    |   INTERGER               { $$ = new column_type(data_type::INTERGER); }
    |   LONG                   { $$ = new column_type(data_type::LONG); }
    |   FLOAT                  { $$ = new column_type(data_type::FLOAT); }
    |   DOUBLE                 { $$ = new column_type(data_type::DOUBLE); }
    |   VARCHAR '(' INTVAL ')' { $$ = new column_type(data_type::VARCHAR, $3); }
    |   CHAR '(' INTVAL ')'    { $$ = new column_type(data_type::CHAR, $3); }
    |   TEXT                   { $$ = new column_type(data_type::TEXT); }
    ;

opt_column_nullable:
		NULL { $$ = true; }
	|	NOT NULL { $$ = false; }
	|	/* empty */ { $$ = false; }
	;

drop_stmt:
        DROP TABLE table_name {
            $$ = new drop_statement(drop_type::DROP_TABLE);
            $$->schema_name = $3.schema_name;
			$$->table_name = $3.table_name;
        }
    ;

delete_stmt:
        DELETE FROM table_name opt_where {
            $$ = new delete_statement();
            $$->schema_name = $3.schema_name;
            $$->table_name = $3.table_name;
            $$->where = $4;
        }
    ;

truncate_stmt:
        TRUNCATE table_name {
            $$ = new delete_statement();
            $$->schema_name = $3.schema_name;
            $$->table_name = $3.table_name;
        }
    ;

insert_stmt:
        INSERT INTO table_name opt_column_list VALUES '(' literal_list ')' {
            $$ = new insert_statement();
            $$->schema_name = $3.schema_name;
            $$->table_name = $3.table_name;
            $$->columns = $4;
            $$->values = $7;
        }
    ;

opt_column_list:
		'(' ident_commalist ')' { 
            $$ = $2; 
        }
	|	/* empty */ { 
            $$ = nullptr; 
        }
	;

ident_commalist:
		IDENTIFIER { 
            $$ = new std::vector<char*>(); 
            $$->push_back($1); 
        }
	|	ident_commalist ',' IDENTIFIER { 
            $1->push_back($3); 
            $$ = $1; 
        }
	;

update_stmt:
        UPDATE table_name SET update_clause_commalist opt_where {
            $$ = new update_statement();
            $$->table_name = $2.table_name;
            $$->updates = $4;
            $$->where = $5;
        }
    ;

update_clause_commalist:
		update_clause { 
            $$ = new std::vector<update_clause *>(); 
            $$->push_back($1); 
        }
	|	update_clause_commalist ',' update_clause { 
            $1->push_back($3); 
            $$ = $1; 
        }
	;

update_clause:
		IDENTIFIER '=' expr {
            $$ = new update_clause();
            $$->column = $1;
            $$->value = $3;
        }
	;

select_stmt:
        SELECT select_list FROM table_name opt_where {
            $$ = new select_statement();
            $$->select_list = $2;
            $$->schema_name = $4->schema_name;
            $$->table_name = $4->table_name;
            $$->where = $5;
        }
    ;

select_list:
		expr_list
	;

expr_list:
		expr { 
            $$ = new std::vector<expression *>(); 
            $$->push_back($1); 
        }
	|	expr_list ',' expr { 
            $1->push_back($3); 
            $$ = $1; 
        }
	;

literal_list:
        literal { 
            $$ = new std::vector<expression *>(); 
            $$->push_back($1); 
        }
    |   literal_list ',' literal { 
            $1->push_back($3); 
            $$ = $1; 
        }
    ;

literal:
		string_literal
	|	bool_literal
	|	num_literal
	|	null_literal
	;

string_literal:
		STRING {  
            $$ = expression::make_literal($1);
        }
	;

bool_literal:
		TRUE { 
            $$ = expression::make_literal(true);
        }
	|	FALSE { 
            $$ = expression::make_literal(false);
        }
	;

num_literal:
		FLOATVAL { 
            $$ = expression::make_literal($1);
        }
	|	int_literal
	;

int_literal:
		INTVAL { 
            $$ = expression::make_literal($1);
        }
	;

null_literal:
	    NULL { 
            $$ = expression::make_null_literal();
        }
	;

opt_semicolon:
        ';'
    |   /* empty */
    ;

opt_where:
		WHERE expr { 
            $$ = $2; 
        }
	|	/* empty */ { 
            $$ = nullptr; 
        }
	;

table_name:
        IDENTIFIER { 
            $$.schema_name = nullptr; 
            $$.table_name = $1; 
        }
    |   IDENTIFIER '.' IDENTIFIER {
            $$.schema_name = $1;
            $$.table_name = $3;
        }
    ;

expr:
		operand
	|	between_expr
	;

operand:
		'(' expr ')' { $$ = $2; }
	|	scalar_expr
	|	comp_expr
	;

between_expr:
		operand BETWEEN operand AND operand { 
            $$ = expression::make_between($1, $3, $5);
        }
	;

scalar_expr:
		column_name
    |   star_expr
	|	literal
	;

star_expr:
        '*' { 
            $$ = new expression(expression_type::STAR); 
        }
    ;

column_name:
		IDENTIFIER { 
            $$ = expression::make_column_ref($1); 
        }
	|	IDENTIFIER '.' IDENTIFIER { 
            $$ = expression::make_column_ref($1, $3); 
        }
	;

comp_expr:
		operand '=' operand			{ 
            $$ = expression::make_op_binary($1, operator_type::OP_EQUALS, $3);
        }
	|	operand EQUALS operand		{
            $$ = expression::make_op_binary($1, operator_type::OP_EQUALS, $3);
        }
	|	operand NOTEQUALS operand	{ 
            $$ = expression::make_op_binary($1, operator_type::OP_NOT_EQUALS, $3);
        }
	|	operand '<' operand			{ 
            $$ = expression::make_op_binary($1, operator_type::OP_LESS, $3);
        }
	|	operand '>' operand			{ 
            $$ = expression::make_op_binary($1, operator_type::OP_GREATER, $3);
        }
	|	operand LESSEQ operand		{  
            $$ = expression::make_op_binary($1, operator_type::OP_LESS_EQUAL, $3);
        }
	|	operand GREATEREQ operand	{ 
            $$ = expression::make_op_binary($1, operator_type::OP_GREATER_EQUAL, $3);
        }
	;
%%