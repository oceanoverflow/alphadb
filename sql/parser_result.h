#pragma once

#include <vector>
#include "statement.h"
#include "statement/create_statement.h"
#include "statement/delete_statement.h"
#include "statement/drop_statement.h"
#include "statement/insert_statement.h"
#include "statement/select_statement.h"
#include "statement/show_statement.h"
#include "statement/update_statement.h"

class parser_result
{
private:
    bool is_valid_;
    char* error_msg_;
    int error_line_;
    int error_column_;
    std::vector<statement *> statements_;
public:
    parser_result();
    parser_result(statement* stmt);
    parser_result(parser_result&& moved);
    virtual ~parser_result();

    void set_valid(bool flag);
    bool is_valid() const;
    size_t size() const;

    void set_error(char* error_msg, int error_line, int error_column);
    const char* error_msg() const;
    int error_line() const;
    int error_column() const;

    void reset();
    void add_statement(statement* stmt);
    const statement* get_statement(int idx) const;
    statement* get_mutable_statement(int idx);
    const std::vector<statement *> get_statements() const;
};
