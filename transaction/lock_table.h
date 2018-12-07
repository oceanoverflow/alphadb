#pragma once

#include <mutex>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

typedef unsigned long txn_id_t;

typedef void* data_item;

struct lock_entry
{
    std::mutex mutex;
    std::condition_variable cond_var;
    
    bool locked;
    std::atomic<int> cnt;
    txn_id_t txn_id;
    data_item item;
    
    lock_entry* next;
    lock_entry* prev;

    lock_entry();
    ~lock_entry();
};

lock_entry::lock_entry(): locked{false}, cnt{0} {}

class lock_table
{
    friend class lock_manager;
private:
    std::mutex mutex_;
    int table_size_;
    lock_entry** table_;

    int hash(data_item item) const;
public:
    lock_table(int size = 1024);
    ~lock_table();

    lock_entry* search(data_item item);
    lock_entry* add(data_item item);
    bool remove(data_item item);
};