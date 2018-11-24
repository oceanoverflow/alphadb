#pragma once

#include <vector>
#include "statement.h"
#include "expression.h"

// INSERT INTO teachers VALUES ('SHIZHONGZHAN', 50);

struct insert_statement: statement
{
    insert_statement();
    virtual ~insert_statement();
    char* schema_name;
    char* table_name;
    std::vector<char *>* columns;
    std::vector<expression *>* values;
};

insert_statement::insert_statement(): 
    statement(statement_type::INSERT),
    schema_name{nullptr},
    table_name{nullptr},
    columns{nullptr},
    values{nullptr} {}

insert_statement::~insert_statement() {
    free(schema_name);
    free(table_name);

    if (columns != nullptr) {
        for(auto&& cs: *columns)
        {
            free(cs);
        }
        delete columns;  
    }

    if (values != nullptr) {
        for(auto&& v : *values)
        {
            delete v;
        }
        delete values;
    }
}

