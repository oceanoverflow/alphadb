#pragma once

#include "rid.h"
#include "file_handle.h"
#include "b_plus_tree.h"

struct index_header
{
    int num_pages;
    int root_page;
    int pair_size;
    int fanout;
    int height;
    int attr_type;
    int attr_length;
};

class index_handle
{
    friend class index_manager;
    friend class b_plus_tree;
private:
    file_handle& file_handle_;
    bool file_is_open_;
    bool header_changed_;
    b_plus_tree* tree_;   
    index_header index_header_; 
public:
    index_handle(file_handle& fh, index_header header);
    ~index_handle();
    index_handle(const index_handle& handle);
    index_handle& operator=(const index_handle& handle);

    friend std::ostream& operator<<(std::ostream& os, const index_handle& tree);

    rid search(void* data);
    void insert_entry(void* data, const rid& id);
    bool delete_entry(void* data, const rid& id);
    
    int allocate_page();
    void pin_page(int page);
    void unpin_page(int page);
    void dispose_page(int page);
    void force_pages();

    void mark_dirty(int page);

    page_handle get_this_page(int page) const;
};
