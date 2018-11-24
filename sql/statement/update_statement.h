#pragma once

#include <vector>

#include "statement.h"
#include "expression.h"

struct update_clause
{
    char* column;
    expression* expr;
};

struct update_statement: statement
{
    update_statement();
    virtual ~update_statement();

    char* schema_name;
    char* table_name;
    std::vector<update_clause *>* updates;
    expression* where;
};

update_statement::update_statement(): 
    statement(statement_type::UPDATE),
    schema_name{nullptr},
    table_name{nullptr},
    updates{nullptr},
    where{nullptr} {}

update_statement::~update_statement() 
{
    delete schema_name;
    delete table_name;
    delete where;

    if (updates != nullptr) {
        for(update_clause* update : *updates)
        {
            free(update->column);
            delete update->expr;
            delete update;
        }
        delete updates;
    }
}