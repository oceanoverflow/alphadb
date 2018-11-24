#pragma once

#include "rid.h"
#include "page_handle.h"
#include "index_format.h"
#include "b_plus_tree.h"

typedef void* key_t;

class leaf_node;
class internal_node;

// typically, a node is made to be the same size 
// as disk block, which is typically 4 kilobytes.
class node
{
    friend class leaf_node;
    friend class b_plus_tree;
    friend class internal_node;
    friend class index_iterator;
protected:
    rid page_id_;
    int fanout_;
    bool in_memory;

    node* parent_;
    b_plus_tree* tree_;
    
    page_handle page_handle_;
    index_format index_format_;
    leaf_node* next_;
protected:
    void pin_node();
    void unpin_node();
    void mark_dirty_and_unpin_node();
public:
    // this constructor is not good, page_handle should be pageid instead
    node(rid page_id, b_plus_tree* tree);
    node(node* parent, rid page_id, b_plus_tree* tree);
    virtual ~node();
    
    int fanout() const;
    node* parent() const;
    void set_parent(node* parent);

    bool is_root() const;
    virtual bool is_leaf() const;

    virtual int size() const;
    virtual int min_size() const;
    virtual int max_size() const;

    virtual const key_t first_key() const; 
};