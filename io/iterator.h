#pragma once

#include "rid.h"
#include <vector>

enum class criterion_type
{
    EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
    RANGE
};

struct criterion
{
    void* equal;
    void* less;
    void* greater;
    criterion_type type;
};

class iterator
{
protected:
    mutable rid now_;
public:
    iterator();
    virtual ~iterator();

    virtual rid begin() const = 0;
    virtual rid end() const = 0;
    virtual rid next() const = 0;

    virtual bool has_next() const = 0;

    virtual void reset() = 0;
protected:
    bool fit(rid id, const std::vector<criterion *>& cs) const;
};
