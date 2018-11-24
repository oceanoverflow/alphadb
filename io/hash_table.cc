#include "hash_table.h"

hash_table::hash_table(int bucket_num): bucket_num_{bucket_num}
{
    hash_table_ = new hash_entry* [bucket_num_];
    
    for(int i = 0; i < bucket_num_; i++)
    {
        hash_table_[i] = nullptr;
    }
}

hash_table::~hash_table()
{
    
    for(int i = 0; i < bucket_num_; i++)
    {
        
        if (hash_table_[i] == nullptr) {
            continue;
        } 
        else {
            hash_entry* entry = hash_table_[i];
            
            while(entry != nullptr){
                hash_entry* next = entry->next;
                delete entry;
                entry = next;
            }       
        }
        
    }

    delete[] hash_table_;  
}

int hash_table::hash(int fd, int page) const 
{
    return (fd + page) % bucket_num_;
}

int hash_table::find(int fd, int page) const 
{
    int bucket = hash(fd, page);
    hash_entry* entry = hash_table_[bucket];
    
    while(entry != nullptr){
        
        if (entry->fd == fd && entry->page == page) {
            return entry->slot;
        }
        
        entry = entry->next;
    }

    return -1; 
}

void hash_table::insert(int fd, int page, int slot) 
{
    int exist = find(fd, page);
    
    if (exist != -1) {
        return;
    }

    int bucket = hash(fd, page);
    hash_entry* entry = new hash_entry(fd, page, slot);

    hash_entry* head = hash_table_[bucket];
    hash_table_[bucket] = entry;
    hash_table_[bucket]->prev = nullptr;
    hash_table_[bucket]->next = head;
    
    if (head != nullptr) {
        head->prev = hash_table_[bucket];
    }
    
}

void hash_table::remove(int fd, int page) 
{
    int exist = find(fd, page);
     
    if (exist == -1) {
        return;
    }

    int bucket = hash(fd, page);
    hash_entry* entry = hash_table_[bucket];
    
    while(entry != nullptr){  

        if (entry->fd == fd && entry->page == page) {
            break;
        }

        entry = entry->next; 
    }
    
    
    if (entry == hash_table_[bucket]) {
        hash_table_[bucket] = entry->next;
        delete entry;
        entry = nullptr;
        return;
    }

    
    if (entry->next == nullptr) {
        entry->prev->next = nullptr;
        delete entry;
        entry = nullptr;
        return;
    }

    entry->next->prev = entry->prev;
    entry->prev->next = entry->next;
    delete entry;
    entry = nullptr;
    return;
}

std::ostream& operator<<(std::ostream& os, const hash_table& table)
{
    
    for(int i = 0; i < table.bucket_num_; i++)
    {
        os << i << "|";
        hash_entry* entry = table[i];
        
        while(entry != nullptr){
            os << "<->(" << entry->fd << "," << entry->page << ")";
            entry = entry->next;
        }

        os << std::endl;
    }

    return os;
    
}
