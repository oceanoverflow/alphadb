#include "leaf_node.h"

#include <string>
#include <sstream>

leaf_node::leaf_node(rid page_id, b_plus_tree* tree)
: node(nullptr, page_id, tree) {}

leaf_node::leaf_node(node* parent, rid page_id, b_plus_tree* tree)
: node(parent, page_id, tree) {}

leaf_node::~leaf_node() {}

leaf_node* leaf_node::next() const
{
    return next_;
}

void leaf_node::set_next(leaf_node* next)
{
    index_format_.rids[index_format_.hdr.count-1] = next->page_id_;
    next_ = next;
}

int leaf_node::size() const
{
    return index_format_.hdr.count;
}

// ⌈(n-1)/2⌉
int leaf_node::min_size() const
{
    return tree_->fanout_ % 2 == 0 ? (tree_->fanout_ + 1 ) / 2 : tree_->fanout_ / 2;
}

int leaf_node::max_size() const
{
    return tree_->fanout_ - 1;
}

bool leaf_node::is_leaf() const
{
    return true;
}

void leaf_node::insert(key_t key, const rid& id)
{
    index_format_.insert(key, id);
}

rid leaf_node::lookup(key_t key) const
{
    rid id;
    index_format_.search(key, id);
    return id;
}

bool leaf_node::remove(key_t key, const rid& id)
{
    return index_format_.remove(key, id);
}

const key_t leaf_node::first_key() const
{   
    return index_format_.keys[0];
}

void leaf_node::move_half_to(leaf_node* receiver)
{
    pin_node();
    receiver->pin_node();

    // copy from the this node to the receiver
    for(size_t i = min_size(); i < index_format_.hdr.count; i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        receiver->insert(key, id);
    }
    // pop back the leafnode
    for(size_t i = index_format_.hdr.count-1; i >= min_size(); i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        remove(key, id);
    }

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

void leaf_node::move_all_to(leaf_node* receiver, int index)
{
    pin_node();
    receiver->pin_node();

    for(size_t i = 0; i < index_format_.hdr.count; i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        receiver->insert(key, id);
    }
    receiver->set_next(next());

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
    tree_->index_handle_.dispose_page(page_id_.page());
}

void leaf_node::move_first_to_end_of(leaf_node* receiver)
{
    pin_node();
    receiver->pin_node();

    key_t key = index_format_.keys[0];
    rid id = index_format_.rids[0];
    remove(key, id);
    receiver->insert(key, id);
    // TODO what about the parent
    
    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

void leaf_node::move_last_to_front_of(leaf_node* receiver, int index)
{
    pin_node();
    receiver->pin_node();

    /*
    let m be such that (N'.Pm, N'.Km) is the last pointer/value pair in N'
    remove (N'.Pm, N'.Km) from N'  
    insert (N'.Pm, N'.Km) as the ﬁrst pointer and value in N, 
        by shifting other pointers and values right
    replace K'in parent(N) by N'.Km
    */

    key_t key = index_format_.keys[index_format_.hdr.count-1];
    rid id = index_format_.rids[index_format_.hdr.count-1];
    remove(key, id);
    receiver->insert(key, id);

    // TODO what about the parent

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

std::ostream& operator<<(std::ostream& os, leaf_node& node)
{
    node.pin_node();
    std::ostringstream oss;
    oss << "node count: " << node.index_format_.hdr.count << " | ";
    
    for(int i = 0; i < node.index_format_.hdr.count-1; i++)
    {
        oss << "| pointer: " << node.index_format_.rids[i]
            << "| keys: " << node.index_format_.keys[i]; 
    }

    oss << " | ->" << std::endl;
    
    std::string str = oss.str();
    int len = str.length();

    std::ostringstream ss;

    ss << "+";
    for(int i = 0; i < len; i++)
        ss << "-";
    ss << "+\n";

    std::string str_bar = ss.str();

    os << str_bar << "|" << str << "|\n" << str_bar << std::endl; 
    
    node.unpin_node();
    return os;
}