#pragma once

#include <vector>

enum class statement_type 
{
    CREATE, /// CREATE TABLE students (name STRING, student_number INTEGER, city STRING, grade DOUBLE);
    INSERT, /// INSERT INTO students VALUES ("MAX", 11223344, 'wenzhou', 5.0);
    SELECT, /// SELECT * FROM students WHERE name = "MAX" ;
    UPDATE, /// UPDATE students SET city = 'hangzhou' WHERE name = 'MAX';
    DELETE, /// DELETE FROM students WHERE grade < 3.0;
    DROP,   /// DROP TABLE students; 
    SHOW    /// SHOW TABLES;
};

struct statement
{
    statement(statement_type type);
    virtual ~statement();

    statement_type type() const;
    bool is(statement_type type) const;

    size_t string_length;
private:
    statement_type type_;
};

statement::statement(statement_type type): type_{type} {}

statement::~statement() {}

bool statement::is(statement_type type) const
{
    return type_ == type;
}

statement_type statement::type() const
{
    return type_;
}