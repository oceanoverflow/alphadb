#pragma once

#include "rid.h"
#include "node.h"
#include "leaf_node.h"
#include "internal_node.h"
#include "index_handle.h"

#include <vector>
#include <iostream>

/*
B+ tree update time complexity

insertion: log[n/2]N
deletion:  log[n/2]N
*/

class b_plus_tree
{
    friend class node;
    friend class internal_node;
    friend class leaf_node;
    friend class index_iterator;
private:
    int height_;
    int fanout_;
    // the root node of the tree is usually heavily 
    // accessed and is likey to be in the buffer.
    node* root_;
    index_handle& index_handle_;
public:
    b_plus_tree(const index_handle& ifh);
    ~b_plus_tree();

    friend std::ostream& operator<<(std::ostream& os, const b_plus_tree& tree);

    bool is_empty() const;
    rid  find(key_t k) const;
    std::vector<rid> find_all(key_t k) const;
    // when a record is inserted into, or deleted
    // from a relation, indices on the relation
    // must be updated correspondingly
    void insert(key_t k, const rid& p);
    bool remove(key_t k, const rid& p);
    // updates to a record can be modeled as a deletion of the 
    // old record folllowed by insertion of the updated record
    bool update(key_t k, const rid& oldp, const rid& newp);
    void destroy();

    leaf_node* find_smallest_leaf_node() const; // TODO
    leaf_node* find_largest_leaf_node() const;  // TODO
private:
    void adjust_root();

    node* fetch_node(int page);
    leaf_node* find_leaf_node(key_t k) const;
    void insert_in_leaf(leaf_node* l, key_t k, rid p);
    void insert_in_parent(node* n, key_t k, node* n1);
    bool remove_entry(leaf_node* l, key_t k, rid p);

    // node becomes too large as the result of an insertion
    template<typename N> N* split(N* n);
    // if a node becomes too small ( fewer than ⌈n/2⌉ pointers )
    template<typename N> void coalesce_or_redistribute(N* n);
    template<typename N> void coalesce(N* neighbor, N* n, internal_node* parent, int index);
    template<typename N> void redistribute(N* neighbor, N* n, internal_node* parent, int index);
};