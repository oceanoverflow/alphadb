#pragma once

#include "record.h"
#include "statement.h"

#include <vector>

// CREATE TABLE students (name STRING, student_number INTEGER, city STRING, grade DOUBLE);
// CREATE INDEX ON students.name;

enum class create_type
{
    CREATE_TABLE,
    CREATE_INDEX
};

struct create_statement: statement
{
    create_statement(create_type type);
    virtual ~create_statement();

    create_type type;
    char* schema_name;
    char* table_name;
    char* index_column;
    std::vector<column_definition *>* columns;
};

create_statement::create_statement(create_type type): 
    statement(statement_type::CREATE),
    type{type},
    schema_name{nullptr},
    table_name{nullptr},
    index_column{nullptr},
    columns{nullptr} {}

create_statement::~create_statement() 
{
    free(schema_name);
    free(table_name);
    free(index_column);
    
    if (columns != nullptr) {
        for(auto&& i : *columns)
        {
            delete i;
        }
        delete columns;
    }  
}
