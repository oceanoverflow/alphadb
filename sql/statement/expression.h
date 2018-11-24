#pragma once

#include <vector>

enum class expression_type
{
    LITERAL_FLOAT,
    LITERAL_STRING,
    LITERAL_INT,
    LITERAL_BOOL,
    LITERAL_NULL,
    STAR,
    COLUMN_REF,
    OPERATOR
};

enum class operator_type
{
    OP_NONE,

    OP_BETWEEN,

    OP_EQUALS,
    OP_NOT_EQUALS,
    OP_LESS,
    OP_LESS_EQUAL,
    OP_GREATER,
    OP_GREATER_EQUAL
};

struct expression
{
    expression* expr;
    expression* expr2;
    std::vector<expression *>* expr_list;

    char*   name;
    char*   table;
    float   fval;
    int64_t ival;
    bool    flag;

    expression_type type;
    operator_type   op_type;

    expression(expression_type type);
    virtual ~expression();

    bool is_type(expression_type type) const;
    bool is_literal() const;

    static expression* make(expression_type type);

    static expression* make_between(expression* expr, expression* left, expression* right);

    static expression* make_op_binary(expression* expr1, operator_type op, expression* expr2);

    static expression* make_literal(int64_t val);
    static expression* make_literal(double val); 
    static expression* make_literal(char* val);  
    static expression* make_literal(bool val);   
    static expression* make_null_literal();      

    static expression* make_star(void);          
    static expression* make_star(char* table);   

    static expression* make_column_ref(char* name); 
    static expression* make_column_ref(char* table, char* name);
};
