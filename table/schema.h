#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

typedef int column_id_t;

enum class data_type
{
    UNKNOWN,
    INT, 
    LONG,
    FLOAT, 
    DOUBLE,
    CHAR, 
    VARCHAR
};

struct column_type
{
    column_type() = default;
    column_type(data_type type, int64_t length);
    data_type type;
    int64_t length;
};

struct column_definition
{
    column_definition();
    column_definition(char* name, column_type type, bool nullable);
    virtual ~column_definition();
    char* name;
    column_type type;
    bool nullable;

    friend bool operator==(const column_definition& lhs, const column_definition& rhs);
    friend bool operator!=(const column_definition& lhs, const column_definition& rhs);
};

class schema
{
    friend class row;
    friend class table_manager;
    friend class table_iterator;
private:
    std::vector<column_definition> col_defs_;
    std::unordered_map<std::string, column_id_t> col_name_to_id_;
public:
    schema();
    ~schema();

    void add_column(char* name, column_type type, bool nullable);
};