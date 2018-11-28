#pragma once

#include "index_handle.h"
#include "b_plus_tree.h"
#include "iterator.h"
#include "leaf_node.h"

class index_iterator: public iterator
{
private:
    b_plus_tree* tree_;
    index_handle idx_;
    
    leaf_node* current_leaf_node_;
    criterion criterion_;
public:
    index_iterator(const index_handle& index_handle, const criterion& cri);
    ~index_iterator();

    rid begin() const override;
    rid end() const override;
    rid next() const override;

    bool has_next() const override;

    void reset() override;
private:
    bool fit(rid id, const criterion& cs) const;

    bool can_stop_now(void* now) const;
};
