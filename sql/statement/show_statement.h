#pragma once

#include "statement.h"

// SHOW TABLES;

enum class show_type
{
    COLUMNS,
    TABLES
};

struct show_statement: statement
{
    show_statement(show_type type);
    virtual ~show_statement();

    show_type type;
    char* schema_name;
    char* table_name;
};

show_statement::show_statement(show_type type): 
    statement(statement_type::SHOW), 
    type{type},
    schema_name{nullptr},
    table_name{nullptr}
    {}

show_statement::~show_statement()
{
    free(schema_name);
    free(table_name);
}
