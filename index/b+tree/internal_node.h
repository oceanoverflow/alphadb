#pragma once

#include "rid.h"
#include "node.h"
#include "page_handle.h"

#include <utility>
#include <iostream>

class internal_node : public node
{
    friend class b_plus_tree;
    friend std::ostream& operator<<(std::ostream& os, internal_node& node);
public:
    internal_node();
    internal_node(rid page_id, b_plus_tree* tree);
    internal_node(node* parent, rid page_id, b_plus_tree* tree);
    ~internal_node() override;

    int size() const override;               // DONE
    int min_size() const override;           // DONE
    int max_size() const override;           // DONE
    bool is_leaf() const override;           // DONE
    const key_t first_key() const override;  // DONE

    node* first_child() const;               // DONE
    node* lookup(key_t key) const;           // DONE
    node* neighbor(int index) const;         // DONE
    node* remove_and_return_only_child();    // DONE
    void remove(int index);                  // DONE

    int node_index(node* n) const;           // DONE
    key_t key_at(int index) const;           // DONE
    void set_key_at(int index, key_t key);   // DONE  

    void populate_new_root(node* oldnode, key_t key, node* newnode);   // DONE
    int insert_node_after(node* oldnode, key_t newkey, node* newnode); // DONE
    key_t replace_and_return_first_key();                              // DONE why replace
    
    void move_half_to(internal_node* receiver);                       // DONE
    void move_all_to(internal_node* receiver, int index);             // DONE
    void move_first_to_end_of(internal_node* receiver);               // DONE
    void move_last_to_front_of(internal_node* receiver, int index);   // DONE
};