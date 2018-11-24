#pragma once

#include <utility>

typedef std::pair<void*, size_t> tuple_t;

class comparator
{
public:
    virtual int compare(tuple_t t1, tuple_t t2) const = 0;

    virtual const char* name() const = 0;

    virtual ~comparator();
};