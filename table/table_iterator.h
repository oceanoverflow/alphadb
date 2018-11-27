#pragma once

#include "table_handle.h"
#include "iterator.h"

class table_iterator: public iterator
{
private:
    table_handle tbl_handle_;
    const std::vector<criterion *>& criterions_;
public:
    table_iterator(const table_handle& handle, const std::vector<criterion *>& cs);
    ~table_iterator();

    rid begin() const override;
    rid end() const override;
    rid next() const override;

    bool has_next() const override;

    void reset() override;
private:
    bool fit(tuple_t t, const std::vector<criterion *>& cs) const;
};