#pragma once

#include <iostream>

struct hash_entry {
    int fd;
    int page;
    int slot;
    hash_entry* prev;
    hash_entry* next;

    hash_entry();
    hash_entry(int fd, int page, int slot): fd(fd), page(page), slot(slot) {};
};

class hash_table
{ 
private:
    int bucket_num_;
    hash_entry** hash_table_;
    int hash(int fd, int page) const;
public:
    hash_table(int bucket_num);
    ~hash_table();

    hash_entry* operator[](int i) const { return hash_table_[i]; }
    
    int find(int fd, int page) const;
    void insert(int fd, int page, int slot);
    void remove(int fd, int page);

    friend std::ostream& operator<<(std::ostream& os, const hash_table& table);
};


