#pragma once

#include "rid.h"
#include "node.h"
#include "internal_node.h"
#include "page_handle.h"

#include <utility>
#include <iostream>

/*
For i = 1,2,...,n-1 
pointer Pi points to a file record with search-key value Ki.
Pointer Pn has a special purpose
*/

class leaf_node: public node
{
    friend class b_plus_tree;
    friend std::ostream& operator<<(std::ostream& os, leaf_node& node);
public:
    explicit leaf_node(rid page_id, b_plus_tree* tree);
    explicit leaf_node(node* parent, rid page_id, b_plus_tree* tree);
    ~leaf_node() override;

    leaf_node* next() const;        // DONE
    void set_next(leaf_node* next); // DONE

    int size() const override;      // DONE
    int min_size() const override;  // DONE
    int max_size() const override;  // DONE
    bool is_leaf() const override;  // DONE
    const key_t first_key() const override; // DONE

    void insert(key_t key, const rid& id); // DONE
    rid lookup(key_t key) const;           // DONE
    bool remove(key_t key, const rid& id); // DONE

    void move_half_to(leaf_node* node);                         // DONE
    void move_all_to(leaf_node* node, int /**/);                // DONE
    void move_first_to_end_of(leaf_node* receiver);             // DONE
    void move_last_to_front_of(leaf_node* receiver, int index); // DONE
};