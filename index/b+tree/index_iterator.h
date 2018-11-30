#pragma once

#include "index_handle.h"
#include "b_plus_tree.h"
#include "iterator.h"
#include "leaf_node.h"
#include "comparator.h"

class index_iterator: public iterator
{
private:
    b_plus_tree* tree_;
    index_handle idx_;
    
    bool descending_;
    mutable bool next_is_null_;
    mutable bool found_one_;
    
    leaf_node* start_;
    leaf_node* end_;
    mutable leaf_node* current_;

    criterion criterion_;
    comparator* comparator_;
public:
    index_iterator(const index_handle& index_handle, const criterion& cri, bool desc = false);
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
