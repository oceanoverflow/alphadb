#pragma once

#include "buffer_manager.h"
#include "page_handle.h"
#include <iostream>

const int DEFAULT_PAGE_SIZE{4096};

const int PAGE_LIST_END{-1};

struct file_header
{
    int first_free;
    int page_num;
};

const int PAGE_USED{-2};

struct page_header
{
    int next_free;
};

class io_manager;

const int ALL_PAGES{-1};

class file_handle
{
    friend class io_manager;
private:
    bool is_valid_page(int page) const; // DONE
    int fd_;
    file_header header_;
    buffer_manager* buffer_manager_;
    bool file_is_open_;
    bool header_changed_;
public:
    file_handle();
    ~file_handle();
    file_handle(const file_handle& handle);
    file_handle& operator=(const file_handle& handle);

    page_handle get_first_page() const;
    page_handle get_next_page(int current) const;
    page_handle get_prev_page(int current) const;
    page_handle get_last_page() const;

    page_handle allocate_page();               // DONE
    page_handle get_this_page(int page) const; // DONE  
    void mark_dirty(int page) const;           // DONE
    void unpin_page(int page) const;           // DONE
    void flush_pages();                        // DONE
    void force_page(int page = ALL_PAGES);     // DONE
    void dispose_page(int page);               // DONE

    friend std::ostream& operator<<(std::ostream& os, const file_handle& handle);
};