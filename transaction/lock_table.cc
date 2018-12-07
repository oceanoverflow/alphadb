#include "lock_table.h"

#include <algorithm>

lock_table::lock_table(int size): table_size_{size}
{
    table_ = new lock_entry*[table_size_];
}

lock_table::~lock_table()
{
    for(size_t i = 0; i < table_size_; i++)
    {
        lock_entry* pointer = table_[i];
        while(pointer){
            lock_entry* old = pointer;
            pointer = pointer->next;
            delete old;
            old = nullptr;
        }
    }
    delete [] table_;
}

int lock_table::hash(data_item item) const
{
    return reinterpret_cast<int>(item) % table_size_;
}

lock_entry* lock_table::search(data_item item)
{
    int pos = hash(item);
    lock_entry* pointer = table_[pos];

    while(pointer != nullptr){
        if (item == pointer->item) {
            return pointer;
        }
        pointer = pointer->next;
    }
    return nullptr;
}

lock_entry* lock_table::add(data_item item)
{
    std::unique_lock<std::mutex> lk(mutex_);
    lock_entry* pointer = search(item);
    if (pointer != nullptr) {
        return pointer;
    }
    else {
        int pos = hash(item);
        lock_entry* entry = new lock_entry();
        
        lock_entry* head = table_[pos];
        table_[pos] = entry;
        entry->next = head;
        entry->prev = nullptr;
        head->prev = entry;
        
        return entry;
    }
}

bool lock_table::remove(data_item item)
{
    std::unique_lock<std::mutex> lk(mutex_);
    lock_entry* pointer = search(item);
    if (pointer == nullptr) {
        return false;
    }
    else {        
        int pos = hash(item);
        if (pointer == table_[pos]) {
            table_[pos] = pointer->next;
            delete pointer;
            pointer = nullptr;
            return true;
        }

        if (pointer->next == nullptr) {
            pointer->prev->next = nullptr;
            delete pointer;
            pointer = nullptr;
            return true;
        }

        pointer->next->prev = pointer->prev;
        pointer->prev->next = pointer->next;
        delete pointer;
        pointer = nullptr;
        return true;  
    }
}