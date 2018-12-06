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
    return *reinterpret_cast<int *>(item) % table_size_;
}

bool lock_table::search(data_item item, txn_id_t id) const
{
    int pos = hash(item);
    lock_entry* pointer = table_[pos];

    while(pointer != nullptr){
        if (item == pointer->item) {
            for(auto& pair : pointer->txn_ids)
            {
                if (pair.first == id) {
                    return true;
                }
            }
        }
        pointer = pointer->next;
    }
    return false;
}

bool lock_table::add(data_item item, txn_id_t id, lock_mode mode)
{
    if (search(item, id)) {
        return false;
    }
    else {
        int pos = hash(item);
        lock_entry* entry = new lock_entry();
        entry->txn_ids.push_back({id, mode});
        
        lock_entry* head = table_[pos];
        table_[pos] = entry;
        entry->next = head;
        entry->prev = nullptr;
        head->prev = entry;

        
        if (m_.find(id) == m_.end()) {
            m_[id] = new std::vector<data_item>();
        }
        else {
            m_[id]->push_back(item);
        }
        
        return true;
    }
}

bool lock_table::remove(data_item item, txn_id_t id)
{
    if (!search(item, id)) {
        return false;
    }
    else {
        int pos = hash(item);
        lock_entry* pointer = table_[pos];
        
        while(pointer != nullptr){
            if (pointer->item == item) {
                auto result = std::find(pointer->txn_ids.begin(), pointer->txn_ids.end(), id);
                if (result != std::end(pointer->txn_ids)) {
                    pointer->txn_ids.erase(result);
                }
            }
        }
        auto iter = std::find(m_[id]->begin(), m_[id]->end(), item);
        
        if (iter != m_[id]->end()) {
            m_[id]->erase(iter);
        }
        
        if (pointer->txn_ids.size() == 0) {
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
}