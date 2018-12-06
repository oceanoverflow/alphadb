#pragma once

#include <list>
#include <vector>
#include <utility>
#include <unordered_map>

typedef unsigned long txn_id_t;

typedef void* data_item;

enum class lock_mode
{
    shared,
    exclusive
};

enum class lock_status
{
    granted,
    waiting
};

struct lock_entry
{
    data_item item;
    lock_mode mode;
    std::list<std::pair<txn_id_t, lock_mode>> txn_ids;
    
    lock_entry* next;
    lock_entry* prev;

    lock_entry();
    ~lock_entry();
};

class lock_table
{
    friend class lock_manager;
private:
    int table_size_;
    lock_entry** table_;
    // the lock table should also maintain an index 
    // on transaction identifiers, so that it is 
    // possible to determine efficiently the set
    // of locks held by a given transaction
    std::unordered_map<txn_id_t, std::vector<data_item>*> m_;

    int hash(data_item item) const;
public:
    lock_table(int size = 1024);
    ~lock_table();

    bool search(data_item item, txn_id_t id) const;
    bool add(data_item item, txn_id_t id, lock_mode mode);
    bool remove(data_item item, txn_id_t id);
};