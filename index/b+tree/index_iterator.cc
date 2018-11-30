#include "index_iterator.h"

/*
ascending:
                start
  equal:             |--|
  smallest -------------------------------> largest

                start
  >= >               |--------------------
  smallest -------------------------------> largest

        start
  < <=      ---------|
  smallest -------------------------------> largest
  
descending:
                        start
  equal:             |--|
  smallest -------------------------------> largest

                                          start
  >= >               |--------------------
  smallest -------------------------------> largest

                     start
  < <=      ---------|
  smallest -------------------------------> largest
*/

index_iterator::index_iterator(const index_handle& handle, const criterion& cri, bool desc)
: criterion_{cri}, idx_(handle), descending_{desc}, found_one_{false}, next_is_null_{true}
{
    tree_ = new b_plus_tree(handle);
    comparator_ = new comparator(criterion_.coldef.type.type);
    now_ = rid{-1, -1};

    if (!descending_) {
        if (criterion_.type == criterion_type::GREATER_EQUAL 
            || criterion_.type == criterion_type::GREATER_THAN) {

            start_ = tree_->find_leaf_node(criterion_.val);
            end_ = tree_->find_largest_leaf_node();

        }

        if (criterion_.type == criterion_type::LESS_EQUAL 
            || criterion_.type == criterion_type::LESS_THAN) {
            
            start_ = tree_->find_smallest_leaf_node();
            end_ = tree_->find_leaf_node(criterion_.val);

        }
        
        if (criterion_.type == criterion_type::EQUAL) {
            // the start_ should be leftest value
            end_ = nullptr;
        }

        now_ = rid{start_->page_id_.page(), 0};
    }
    else {
        if (criterion_.type == criterion_type::GREATER_EQUAL 
            || criterion_.type == criterion_type::GREATER_THAN) {

            start_ = tree_->find_largest_leaf_node();
            end_ = tree_->find_leaf_node(criterion_.val);

        }

        if (criterion_.type == criterion_type::LESS_EQUAL 
            || criterion_.type == criterion_type::LESS_THAN) {
            
            start_ = tree_->find_smallest_leaf_node();
            end_ = tree_->find_leaf_node(criterion_.val);

        }

        if (criterion_.type == criterion_type::EQUAL) {
            // the start_ should be rightest value
            end_ = nullptr;
        }

        now_ = rid{start_->page_id_.page(), start_->size()-2};
    }

}

index_iterator::~index_iterator()
{
    delete tree_;
    delete comparator_;
}

rid index_iterator::begin() const
{
    return now_;
}

rid index_iterator::end() const
{
    return rid{-1, -1};
}

// TODO missing descending version
rid index_iterator::next() const
{
    current_ = start_;
    
    for(; !next_is_null_;)
    {
        if (current_->page_id_.page() == end_->page_id_.page()) {
            next_is_null_ = true;
        }

        current_->pin_node();

        for(size_t j = now_.slot(); j < current_->size()-1; j++)
        {
            now_ = rid{now_.page(), j};
            if (criterion_.eval(static_cast<char *>(current_->index_format_.keys[j]))) {
                found_one_ = true;
                return rid{current_->page_id_.page(), j};
            }
            else {
                if (found_one_ && can_stop_now(current_->index_format_.keys[j])) {
                    return rid{-2, -2};
                }
            }
        }
        
        rid next = current_->index_format_.rids[current_->size()-1];
        now_ = next;

        current_->unpin_node();
        delete current_;
        current_ = nullptr;
        current_ = new leaf_node(next, tree_);
    }

    return rid{-1, -1};
}

void index_iterator::reset()
{
    now_ = rid{-1, -1};
}

bool index_iterator::fit(rid id, const criterion& cs) const
{
    key_t k = current_->index_format_.keys[id.slot()];
    return cs.eval(static_cast<char *>(k));
}

bool index_iterator::can_stop_now(void* now) const
{
    if (criterion_.type == criterion_type::NOT_EQUAL) return false;

    int cmp = comparator_->compare({now, criterion_.coldef.type.length}, {criterion_.val, criterion_.coldef.type.length});

    if (criterion_.type == criterion_type::EQUAL && cmp != 0) return true;
    if (criterion_.type == criterion_type::GREATER_EQUAL && cmp < 0 && descending_) return true;
    if (criterion_.type == criterion_type::GREATER_THAN && cmp <= 0 && descending_) return true;
    if (criterion_.type == criterion_type::LESS_EQUAL && cmp > 0 && !descending_) return true;
    if (criterion_.type == criterion_type::LESS_THAN && cmp >= 0 && !descending_) return true;
}