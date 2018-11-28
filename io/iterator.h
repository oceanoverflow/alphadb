#pragma once

#include "rid.h"
#include "schema.h"
#include <vector>

enum class criterion_type
{
    NOT_EQUAL,  // TODO add to code later
    EQUAL,
    LESS_THAN,
    LESS_EQUAL,
    GREATER_THAN,
    GREATER_EQUAL,
};

/*
criterion: WHERE x < 100;
when scanning index or table, we first get the raw pointer to the object

               +--- value should start here
               v
         +-----+----+-------------+
         |     |    |             |
         +-----+----+-------------+
where the criterion itself is stable, it is the value we want to compare
changes all the time, so when designing the criterion class, we need to
specify the variant part which is designated as a void pointer, and based
on the type of the value, we return a boolean based on the comparison result.
*/

struct criterion
{
    criterion_type type;
    void* val;
    column_definition coldef;
    
    criterion(void* val, column_definition coldef, criterion_type type);
    ~criterion();

    bool eval(char* buf) const;
private:
    bool internal_eval(char* buf, criterion_type type) const;
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
