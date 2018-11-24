#pragma once

#include "rid.h"

#include <vector>
#include <iostream>

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
    column_definition(char* name, column_type type, bool nullable);
    virtual ~column_definition();
    char* name;
    column_type type;
    bool nullable;
};

class record
{
private: 
    rid rid_;
    int size_;
    char* data_; 
    std::vector<column_definition> columns_;
public:
    record();
    ~record();

    rid get_rid() const;
    char* data() const;
    int size() const;
    void set(char* data, int size, rid _rid);

    friend std::ostream& operator<<(std::ostream& os, const rid& rid);
};