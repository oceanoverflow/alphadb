#include "index_iterator.h"

index_iterator::index_iterator(const index_handle& handle, const std::vector<criterion *>& cs): criterions_{cs}, idx_(handle)
{
    tree_ = new b_plus_tree(handle);
    now_ = rid{-1, -1};
}

index_iterator::~index_iterator()
{
    delete tree_;
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
    if (!fit(now_, criterions_)) return rid{-1, -1};

    rid ret = now_;
    leaf_node* node = static_cast<leaf_node *>(tree_->fetch_node(now_.page()));
    int count = node->index_format_.hdr.count;
    node->pin_node();
    if (now_.slot() < count-2)
        now_.set(now_.page(), now_.slot() + 1);
    else
        now_.set(node->index_format_.rids[count-1].page(), 0);
    node->unpin_node();

    return ret;
}

void index_iterator::reset()
{
    now_ = rid{-1, -1};
}

bool index_iterator::fit(rid id, const std::vector<criterion *>& cs) const
{
    // get the record from the table handle
    // check if it meet and criterions
    return true;
}