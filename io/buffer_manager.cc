#include "buffer_manager.h"
#include "exceptions.h"

#include <cstring>
#include <unistd.h>

#include <iostream>

/*

                    Summary of I/O buffering

    ^                   +-------------+
    |                   |  User data  |
    |                   +-------------+
    |                          |
   U|                          V
   s|               +----------------------+
   e|               | stdio libarary calls |
   r|               |printf(), fputc(), etc|
    |               +----------------------+
   M|                          |
   e|                          V
   m|                  +----------------+
   o|                  |  stdio buffer  |
   r|                  +----------------+
   y|                          |
    |                          V
    |           +------------------------------+
----+-----------|I/O system calls write(), etc.|---------------
    |           +------------------------------+
    |                          |
   K|                          v
   e|               +---------------------+
   r|               | Kernel buffer cache |
   n|               +---------------------+
   e|                          |
   l|                          V
    |              +------------------------+
   M|              | Kernel-initiated write |
   e|              +------------------------+
   m|                          |
   o|                          V
   r|                   +============+
   y|                   |    Disk    |
    |                   +============+
    v

*/

buffer_manager::buffer_manager(int pool_size, int buffer_size): pool_size_{pool_size}, buffer_size_{buffer_size}
{  
    buffer_pool_ = new buffer_entry[pool_size_];
    hash_table_ = new hash_table(HASH_TABLE_SIZE);

    for(int i = 0; i < pool_size_; i++)
    {
        if ((buffer_pool_[i].data = new char[buffer_size_]) == nullptr)
            throw std::bad_alloc();

        memset((void *)buffer_pool_[i].data, 0, buffer_size_);
        free_list_.push_back(i);
    } 
}

buffer_manager::~buffer_manager()
{
    for(int i = 0; i < pool_size_; i++)
        delete[] buffer_pool_[i].data;

    delete hash_table_;
}


int buffer_manager::internal_alloc()
{
    
    if (!free_list_.empty()) {
        int slot = free_list_.front();
        free_list_.pop_front();
        insert_used(slot);
        return slot;
    } 
    else {
        std::list<int>::reverse_iterator it;
        
        for(it = used_list_.rbegin(); it != used_list_.rend(); ++it)
        {
            if (buffer_pool_[*it].pin == 0)
                break;   
        }
        
        if (it == used_list_.rend()) {
            throw std::exception{};//throw no_buf_error{};
        }

        // fuck, bug cost me one night to find, mind of the erase api
        int slot = *it;
        
        if (buffer_pool_[slot].dirty)
            write_page(buffer_pool_[slot].fd, buffer_pool_[slot].page, buffer_pool_[slot].data);

        hash_table_->remove(buffer_pool_[slot].fd, buffer_pool_[slot].page);
        
        remove(slot);
        insert_used(slot);
        return slot;
        
    }
    
}

void buffer_manager::insert_free(int slot)
{
    free_list_.push_front(slot);
}

void buffer_manager::insert_used(int slot)
{
    used_list_.push_front(slot);
    used_map_.insert({slot, used_list_.begin()});
}

void buffer_manager::remove(int slot)
{
    auto it = used_map_.find(slot);
    
    if (it != used_map_.end()) {
        used_list_.erase(it->second);
        used_map_.erase(slot);
        return;
    }
    
    for (auto it = used_list_.begin(); it != used_list_.end(); ++it) {
       if (*it == slot) {
           used_list_.erase(it);
           break;
       }
    }

}

void buffer_manager::force_page(int fd, int page)
{
    
    for(auto i : used_list_)
    {
        buffer_entry& entry = buffer_pool_[i];
        if (entry.fd == fd && (entry.page == page || page == -1)) {
            
            if (entry.dirty) {
                write_page(fd, page, entry.data);
                entry.dirty = false;
            }
            
        }
        
    }
    
}

void buffer_manager::read_page(int fd, int page, char *dest)
{
    off_t offset = page * this->buffer_size_ + 4096;
    
    if (lseek(fd, offset, SEEK_SET) < 0) {
        throw std::exception{};//throw unix_error{};
    }

    int num_bytes = read(fd, dest, this->buffer_size_);
   
    if (num_bytes < 0) {
        throw std::exception{};//throw unix_error{};
    }
    else if (num_bytes != this->buffer_size_) {
        throw std::exception{};//throw incomplete_read_error{};
    }
    
}

void buffer_manager::write_page(int fd, int page, const char *src)
{
    off_t offset = page * this->buffer_size_ + 4096;
    
    if (lseek(fd, offset, SEEK_SET) < 0) {
        throw std::exception{};//throw unix_error{};
    }

    int num_bytes = write(fd, src, this->buffer_size_);
    
    if (num_bytes < 0) {
        throw std::exception{};//throw unix_error{};
    }
    else if (num_bytes != this->buffer_size_) {
        throw std::exception{};//throw incomplete_write_error{};
    }
    
}

void buffer_manager::init_page_info(int fd, int page, int slot)
{
    buffer_pool_[slot].fd = fd;
    buffer_pool_[slot].page = page;
    buffer_pool_[slot].pin = 1;
    buffer_pool_[slot].dirty = false;
}

void buffer_manager::mark_dirty(int fd, int page)
{
    int slot = hash_table_->find(fd, page);
    
    if (slot == -1) {
        throw std::exception{};//throw page_not_in_buf_exception{};
    }

    // TODO why does pin count matter

    buffer_pool_[slot].dirty = true;

    remove(slot);
    insert_used(slot);
}

void buffer_manager::flush_pages(int fd)
{
    
    for(int i = 0; i < pool_size_; i++)
    {   
        buffer_entry& entry = buffer_pool_[i];
        
        
        if (entry.fd == fd) {
            
            if (entry.pin > 0) {
                throw std::exception{};//throw page_pinned_exception{};
            }   
            else {
                
                if (entry.dirty) {
                    write_page(entry.fd, entry.page, entry.data);
                    entry.dirty = false;
                }

                hash_table_->remove(entry.fd, entry.page);

                remove(i);
                insert_free(i);
                
            }   
            
        }    

    }
    
}

char* buffer_manager::get_page(int fd, int page, bool multiple_pins)
{
    int slot = hash_table_->find(fd, page);
    
    if (slot == -1) {
        slot = internal_alloc();
        if (slot == -1) return nullptr;
        
        read_page(fd, page, buffer_pool_[slot].data);
        hash_table_->insert(fd, page, slot);
        init_page_info(fd, page, slot);
    }   
    else {
        
        if (!multiple_pins && buffer_pool_[slot].pin > 0) {
            throw std::exception{};//throw page_pinned_exception{};
        }

        buffer_pool_[slot].pin++;
        remove(slot);
        insert_used(slot);
    }

    return buffer_pool_[slot].data;
    
}

char* buffer_manager::allocate_page(int fd, int page)
{
    int slot = hash_table_->find(fd, page);
    
    if (slot != -1) {
        return nullptr;
        throw std::exception{}; // TODO remove
    }

    slot = internal_alloc();

    hash_table_->insert(fd, page, slot);
    init_page_info(fd, page, slot);

    return buffer_pool_[slot].data;
    
}

void buffer_manager::unpin_page(int fd, int page)
{
    int slot = hash_table_->find(fd, page);
    
    if (slot == -1) {
        throw std::exception{};//throw page_not_in_buf_exception{};     
    }
    
    if (buffer_pool_[slot].pin == 0) {
        throw std::exception{};//throw page_already_unpinned_exception{};
    }
    
    // WTF simple knowledge
    if (--(buffer_pool_[slot].pin) == 0) {
        remove(slot);
        insert_used(slot);
    }
    
}

std::ostream& operator<<(std::ostream& os, const buffer_manager& mgr)
{
    
    os << "---------------------------------------------------------------" << std::endl;
    for(int i = 0; i < mgr.pool_size_; i++)
    {
        os << i << " ::";
        os << " fd = " << mgr.buffer_pool_[i].fd;
        os << " page = " << mgr.buffer_pool_[i].page;
        os << " dirty = " << mgr.buffer_pool_[i].dirty;
        os << " pin = " << mgr.buffer_pool_[i].pin;
        os << " pointer = " << (void *)mgr.buffer_pool_[i].data;
        
        os << std::endl;
        os << "---------------------------------------------------------------" << std::endl;
    }

    os << "used list";
    for(auto&& i : mgr.used_list_)
    {
        os << "->" << i ;
    }

    os << std::endl;

    os << "free list";
    for(auto&& i : mgr.free_list_)
    {
        os << "->" << i;
    }

    os << std::endl;
    
    return os;
    
}