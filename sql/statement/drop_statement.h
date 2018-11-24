#pragma once

#include "statement.h"

enum class drop_type
{
    DROP_TABLE,
    DROP_SCHEMA,
    DROP_INDEX
};

struct drop_statement: statement
{
    drop_statement(drop_type type);
    virtual ~drop_statement();

    drop_type type;
    char* schema_name;
    char* table_name;
};

drop_statement::drop_statement(drop_type type): 
    statement(statement_type::DROP), 
    type{type},
    schema_name{nullptr},
    table_name{nullptr} 
    {}

drop_statement::~drop_statement() 
{
    free(schema_name);
    free(table_name);
}