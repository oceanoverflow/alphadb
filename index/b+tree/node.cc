#include "node.h"

node::node(rid page_id, b_plus_tree* tree)
: parent_{nullptr}, page_id_{page_id}, tree_{tree} {
    index_format_.base = page_handle_.data() + sizeof(index_page_hdr);
}

node::node(node* parent, rid page_id, b_plus_tree* tree) : parent_{parent}, page_id_{page_id}, tree_{tree} {}

node::~node() {}

int node::fanout() const
{
    return tree_->fanout_;
}

node* node::parent() const
{
    return parent_;
}

void node::set_parent(node* parent)
{
    parent_ = parent;
}

bool node::is_root() const
{
    return parent_ == nullptr;
}

void node::pin_node()
{
    page_handle ph = tree_->index_handle_.get_this_page(page_id_.page());
    index_format_.base = page_handle_.data() + sizeof(index_page_hdr);
}

void node::unpin_node()
{ 
    tree_->index_handle_.unpin_page(page_id_.page());
}

void node::mark_dirty_and_unpin_node()
{ 
    tree_->index_handle_.mark_dirty(page_id_.page());
    tree_->index_handle_.unpin_page(page_id_.page());
}