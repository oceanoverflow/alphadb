#pragma once

#include "index_handle.h"
#include "b_plus_tree.h"
#include "iterator.h"

class index_iterator: public iterator
{
private:
    b_plus_tree* tree_;
    index_handle idx_;
    
    const std::vector<criterion *>& criterions_;
public:
    index_iterator(const index_handle& index_handle, const std::vector<criterion *>& cs);
    ~index_iterator();

    rid begin() const override;
    rid end() const override;
    rid next() const override;

    bool has_next() const override;

    void reset() override;
private:
    bool fit(rid id, const std::vector<criterion *>& cs) const;
};
