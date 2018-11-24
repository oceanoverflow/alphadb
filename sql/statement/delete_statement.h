#pragma once

#include "expression.h"
#include "statement.h"

// DELETE FROM students WHERE grade < 3.0;
struct delete_statement: statement
{
    delete_statement();
    virtual ~delete_statement();

    char* schema_name;
    char* table_name;
    expression* where;
};

delete_statement::delete_statement(): 
    statement(statement_type::DELETE),
    schema_name{nullptr},
    table_name{nullptr},
    where{nullptr} {}

delete_statement::~delete_statement() 
{
    free(schema_name);
    free(table_name);
    delete where;
}