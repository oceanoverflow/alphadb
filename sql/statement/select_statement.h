#pragma once

#include <vector>

#include "statement.h"
#include "expression.h"

struct select_statement: statement
{
    select_statement();
    virtual ~select_statement();

    char* schema_name;
    char* table_name;
    std::vector<expression *>* select_list;
    expression* where;
};

select_statement::select_statement(): 
    statement(statement_type::SELECT),
    schema_name{nullptr},
    table_name{nullptr},
    select_list{nullptr},
    where{nullptr}
    {}

select_statement::~select_statement() 
{
    free(schema_name);
    free(table_name);
    delete where;

    if (select_list != nullptr) {
        for(auto&& i : *select_list)
            delete i;
        delete select_list;
    }
}