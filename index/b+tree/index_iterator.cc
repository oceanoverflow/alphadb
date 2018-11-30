#include "index_iterator.h"

/*
  equal:             |--|
  smallest -------------------------------> largest

  >= >               |--------------------
  smallest -------------------------------> largest

  < <=      ---------|
  smallest -------------------------------> largest

*/

index_iterator::index_iterator(const index_handle& handle, const criterion& cri, bool desc)
: criterion_{cri}, idx_(handle), descending_{desc}, found_one_{false}
{
    tree_ = new b_plus_tree(handle);
    comparator_ = new comparator(criterion_.coldef.type.type);
    now_ = rid{-1, -1};
}

index_iterator::~index_iterator()
{
    delete tree_;
    delete comparator_;
}

rid index_iterator::begin() const
{
    // return the first rid that meet the criterion
    return now_;
}

rid index_iterator::end() const
{
    return rid{-1, -1};
}

rid index_iterator::next() const
{
    // if (!fit(now_, criterion_)) return rid{-1, -1};

    // rid ret = now_;
    // leaf_node* node = static_cast<leaf_node *>(tree_->fetch_node(now_.page()));
    // int count = node->index_format_.hdr.count;
    // node->pin_node();
    // if (now_.slot() < count-2)
    //     now_.set(now_.page(), now_.slot() + 1);
    // else
    //     now_.set(node->index_format_.rids[count-1].page(), 0);
    // node->unpin_node();

    // return ret;

    // should have two for loop, the first one is used for
    // looping through blocks, the second one in used for
    // looping in the block


    
    for(; current_leaf_node_ != nullptr;)
    {
        
        
        
        // next leaf node
    }
    
}

void index_iterator::reset()
{
    now_ = rid{-1, -1};
}

bool index_iterator::fit(rid id, const criterion& cs) const
{
    key_t k = current_leaf_node_->index_format_.keys[id.slot()];
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