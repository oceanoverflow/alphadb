#include "parser_result.h"

parser_result::parser_result(): is_valid_{false}, error_msg_{nullptr}
{
}

parser_result::parser_result(statement* stmt): is_valid_{false}, error_msg_{nullptr}
{
    add_statement(stmt);
}

parser_result::parser_result(parser_result&& moved)
{
    statements_ = std::move(moved.statements_);
    is_valid_ = moved.is_valid_;
    error_msg_ = moved.error_msg_;
}

parser_result::~parser_result() {
    reset();
}

void parser_result::set_valid(bool flag)
{
    is_valid_ = flag;
}

bool parser_result::is_valid() const
{
    return is_valid_;
}

size_t parser_result::size() const
{
    return statements_.size();
}

void parser_result::set_error(char* error_msg, int error_line, int error_column)
{
    error_msg_ = error_msg;
    error_line_ = error_line;
    error_column_ = error_column;
}

const char* parser_result::error_msg() const
{
    return error_msg_;
}

int parser_result::error_line() const
{
    return error_line_;
}

int parser_result::error_column() const
{
    return error_column_;
}

void parser_result::reset()
{
    is_valid_ = false;
    free(error_msg_);
    error_msg_ = nullptr;
    error_line_ = -1;
    error_column_ = -1;
    
    for(auto&& stmt : statements_)
        delete stmt;
    
    statements_.clear();
}

void parser_result::add_statement(statement* stmt)
{
    statements_.push_back(stmt);
}

const statement* parser_result::get_statement(int idx) const
{
    return statements_[idx];
}

statement* parser_result::get_mutable_statement(int idx)
{
    return statements_[idx];
}

const std::vector<statement *> parser_result::get_statements() const
{
    return statements_;
}