#pragma once

#include "hash_table.h"

#include <list>
#include <unordered_map>
#include <iostream>

const int HASH_TABLE_SIZE{20};

class buffer_manager
{
    struct buffer_entry
    {
        bool dirty;
        int fd;
        int page;
        char* data;
        int pin;
    };
private:

    int pool_size_;
    int buffer_size_;
    
    hash_table* hash_table_;
    buffer_entry* buffer_pool_;
    
    std::list<int> free_list_;

    std::list<int> used_list_;
    std::unordered_map<int, std::list<int>::iterator> used_map_;

public:
    buffer_manager(int pool_size = 4, int buffer_size = 4096);
    ~buffer_manager();
    buffer_manager(const buffer_manager& mgr) = delete;
    buffer_manager& operator=(const buffer_manager& mgr) = delete;

    friend std::ostream& operator<<(std::ostream& os, const buffer_manager& mgr);

    char* get_page(int fd, int page, bool multiple_pins = true); // DONE
    char* allocate_page(int fd, int page);
    void force_page(int fd, int page);
    void flush_pages(int fd);
    void unpin_page(int fd, int page);
    void mark_dirty(int fd, int page);
private:

    int internal_alloc();
    void insert_free(int slot);
    void insert_used(int slot);
    void remove(int slot);
    
    void read_page(int fd, int page, char *dest);
    void write_page(int fd, int page, const char *source);
    void init_page_info(int fd, int page, int slot);
};

