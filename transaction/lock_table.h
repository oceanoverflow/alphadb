#pragma once

#include <list>
#include <utility>
#include <mutex>
#include <atomic>
#include <unordered_map>
#include <condition_variable>

typedef unsigned long txn_id_t;

typedef void* data_item;

enum class lock_status
{
    granted,
    waiting
};

enum class lock_mode
{
    none,
    shared,
    exclusive
};

/*
        lock compatibility matrix
+---------+---------------+----------------+
|         | txn2 requests |  txn2 requests |
|         |  shared lock  | exclusive lock |
+---------+---------------+----------------+
|   txn1  |               |                |
|  holds  |    proceed    |    proceed     |
| no lock |               |                |
+---------+---------------+----------------+
|   txn1  |               |                |
|  holds  |    proceed    |      wait      |
| shared  |               |                |
|   lock  |               |                |
+---------+---------------+----------------+
|   txn1  |               |                |
|  holds  |     wait      |      wait      |
|exclusive|               |                |
|   lock  |               |                |
+---------+---------------+----------------+

locks are associated with each data item. A txn must acquire a read (shared) or write (exclusive) lock
on an item in order to read or write it within the txn. You must acquire a write lock on an item if you
intend to charge it even if your only read it first.

a write lock on an item conflicts with all other locks (read or write) on the item; 
a read lock conflicts only with a write lock.

If T1 requests a lock on x and T2 holds a conflicting lock on x, T1 must wait.
*/

struct lock_entry
{
    std::mutex mutex;
    std::condition_variable cond_var;

    std::list<std::tuple<txn_id_t, lock_mode, lock_status>> wait_list;
    lock_mode current_lock_mode;
    int shared_lock_cnt;
    data_item item;
    
    lock_entry* next;
    lock_entry* prev;

    lock_entry();
    ~lock_entry();
};

lock_entry::lock_entry() {}

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