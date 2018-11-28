#pragma once

#include "schema.h"
#include <utility>

typedef std::pair<void*, size_t> tuple_t;

class comparator
{
public:
    comparator(data_type type);
    ~comparator();

    int compare(tuple_t t1, tuple_t t2) const;
private:
    data_type data_type_;
};