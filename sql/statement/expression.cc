#include "expression.h"

expression::expression(expression_type type):
 type{type}, expr{nullptr}, expr2{nullptr}, expr_list{nullptr}, name{nullptr}, table{nullptr}, fval{0}, ival{0}, op_type{operator_type::OP_NONE}
{

}

expression::~expression()
{
    
}

expression* expression::make(expression_type type)
{
    expression* e = new expression(type);
    return e;
}

expression* expression::make_between(expression* expr, expression* left, expression* right)
{
    expression* e = new expression(expression_type::OPERATOR);
    e->op_type = operator_type::OP_BETWEEN;
    e->expr_list = new std::vector<expression *>();
    e->expr_list->push_back(left);
    e->expr_list->push_back(right);
    return e;
}

expression* expression::make_star(void)
{
    expression* e = new expression(expression_type::STAR);
    return e;
}

expression* expression::make_star(char* table)
{
    expression* e = new expression(expression_type::STAR);
    e->table = table;
    return e;
}

expression* expression::make_column_ref(char* name)
{
    expression* e = new expression(expression_type::COLUMN_REF);
    e->name = name;
    return e;
}

expression* expression::make_column_ref(char* table, char* name)
{
    expression* e = new expression(expression_type::COLUMN_REF);
    e->table = name;
    e->name = name;
    return e;
}

expression* expression::make_literal(int64_t val)
{
    expression* e = new expression(expression_type::LITERAL_INT);
    e->ival = val;
    return e;
}

expression* expression::make_literal(double val)
{
    expression* e = new expression(expression_type::LITERAL_FLOAT);
    e->fval = val;
    return e;
}

expression* expression::make_literal(bool val)
{
    expression* e = new expression(expression_type::LITERAL_BOOL);
    e->flag = val;
    return e;
}

expression* expression::make_literal(char* val)
{
    expression* e = new expression(expression_type::LITERAL_STRING);
    e->name = val;
    return e;
}

expression* expression::make_null_literal()
{
    expression* e = new expression(expression_type::LITERAL_NULL);
    return e;
}

expression* make_op_binary(expression* expr1, operator_type op, expression* expr2)
{
    expression* e = new expression(expression_type::OPERATOR);
    e->op_type = op;
    e->expr = expr1;
    e->expr2 = expr2;
    return e;
}