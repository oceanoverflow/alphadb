#include "internal_node.h"

#include <sstream>

internal_node::internal_node(rid page_id, b_plus_tree* tree) : node(nullptr, page_id, tree) {}

internal_node::internal_node(node* parent, rid page_id, b_plus_tree* tree) : node(parent, page_id, tree) {}

internal_node::~internal_node() {}

bool internal_node::is_leaf() const
{
    return false;
}

int internal_node::size() const
{
    // Includes the first entry, which has key DUMMY_KEY and a value that
    // points to the left of first postive key k1 (i.e. a node whose keys are all < k1)
    return index_format_.hdr.count;
}

// ⌈n/2⌉
int internal_node::min_size() const
{
    return (tree_->fanout_ + 1) / 2;
}   

int internal_node::max_size() const
{
    return tree_->fanout_;
}

const key_t internal_node::first_key() const
{   
    return index_format_.keys[0];
}

int internal_node::node_index(node* n) const
{
    rid id = n->page_id_;
    
    for(size_t i = 0; i < index_format_.hdr.count; i++)
        if (index_format_.rids[i].page() == id.page())
            return i;
    
    return -1;
}

void internal_node::remove(int index)
{
    key_t key = index_format_.keys[index];
    rid id = index_format_.rids[index];
    index_format_.remove(key, id);
}

key_t internal_node::key_at(int index) const
{
    return index_format_.keys[index];
}

void internal_node::set_key_at(int index, key_t key)
{
    index_format_.keys[index] = key;
}

void internal_node::move_half_to(internal_node* receiver)
{
    pin_node();
    receiver->pin_node();

    for(size_t i = min_size(); i < index_format_.hdr.count; i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        receiver->index_format_.insert(key, id);
    }

    for(size_t i = index_format_.hdr.count-1; i >= min_size(); i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        index_format_.remove(key, id);
    }

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

void internal_node::move_all_to(internal_node* receiver, int index)
{
    pin_node();
    receiver->pin_node();

    index_format_.keys[0] = static_cast<internal_node *>(parent())->key_at(index);
    
    for(size_t i = 0; i < index_format_.hdr.count; i++)
    {
        key_t key = index_format_.keys[i];
        rid id = index_format_.rids[i];
        receiver->index_format_.insert(key, id);
    }

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

void internal_node::move_first_to_end_of(internal_node* receiver)
{
    pin_node();
    receiver->pin_node();

    // receiver <- internal
    key_t key = index_format_.keys[0];
    rid id = index_format_.rids[0];

    receiver->index_format_.insert(key, id);
    index_format_.remove(key, id);

    static_cast<internal_node *>(parent())->set_key_at(1, index_format_.keys[0]);

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

void internal_node::move_last_to_front_of(internal_node* receiver, int index)
{
    pin_node();
    receiver->pin_node();

    /*
    if (N is a nonleaf node) then begin 
        let m be such that N'.Pm is the last pointer in N'
        remove (N'.Km−1, N'.Pm) from N'  
        insert (N'.Pm, K') as the ﬁrst pointer and value in N, 
            by shifting other pointers and values right 
        replace K'in parent(N) by N'.K m−1
    end
    */

    // internal -> receiver
    key_t key = index_format_.keys[index_format_.hdr.count-1];
    rid id = index_format_.rids[index_format_.hdr.count-1];
    index_format_.remove(key, id);

    receiver->set_key_at(0, static_cast<internal_node *>(parent())->key_at(index));
    receiver->index_format_.insert(key, id);
    static_cast<internal_node *>(parent())->set_key_at(index, nullptr);

    receiver->mark_dirty_and_unpin_node();
    mark_dirty_and_unpin_node();
}

node* internal_node::first_child() const
{
    rid id = index_format_.rids[0];
    return new node(id, tree_);
}

node* internal_node::lookup(key_t key) const
{
    int idx = index_format_.find_in_internal(key);
    rid id = index_format_.rids[idx];
    return new node(id, tree_);
}

node* internal_node::neighbor(int index) const
{
    rid id = index_format_.rids[index];
    return new node(id, tree_);
}

node* internal_node::remove_and_return_only_child()
{
    rid id = index_format_.rids[0];
    return new node(id, tree_);
}

key_t internal_node::replace_and_return_first_key()
{
    key_t key = index_format_.keys[0];
    index_format_.keys[0] = nullptr;
    return key;
}

void internal_node::populate_new_root(node* oldnode, key_t key, node* newnode)
{
    index_format_.insert(nullptr, oldnode->page_id_);
    index_format_.insert(key, newnode->page_id_);
    tree_->height_++;
}

int internal_node::insert_node_after(node* oldnode, key_t newkey, node* newnode)
{
    int pos;
    for(size_t i = 0; i < index_format_.hdr.count; i++) 
    {
        if (index_format_.rids[i] == oldnode->page_id_)
        {
            pos = i + 1;
            break;
        }
    }
    //  TODO insert newkey and newnode at position 
    return index_format_.hdr.count;
}

std::ostream& operator<<(std::ostream& os, internal_node& node)
{
    node.pin_node();
    std::ostringstream oss;
    oss << "node count: " << node.index_format_.hdr.count << " | ";
    
    for(int i = 0; i < node.index_format_.hdr.count-1; i++)
    {
        oss << "| pointer: " << node.index_format_.rids[i]
            << "| keys: " << node.index_format_.keys[i]; 
    }

    oss << " | pointer: " << node.index_format_.rids[node.size()-1] << std::endl;
    
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