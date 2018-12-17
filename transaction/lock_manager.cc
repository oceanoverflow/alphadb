#include "lock_manager.h"

#include <vector>

lock_manager::lock_manager()
{
    lock_table_ = new lock_table();
}

lock_manager::~lock_manager()
{
    delete lock_table_;
}

/*
When a lock request message arrives, it adds a record to the end of the linked
list for the data item, if the linked list is present. Otherwise it creates a new
linked list, containing only the record for the request.

It always grants a lock request on a data item that is not currently locked.
But if the transaction requests a lock on an item on which a lock is currently
held, the lock manager grants the request only if it is compatible with the lock
that are currently held, and all earlier requests have been granted already.
Otherwise the request has to wait.
*/
void lock_manager::lock(txn_id_t txn_id, lock_mode mode, data_item item)
{
    lock_entry* entry = lock_table_->add(item);
    {
        std::unique_lock<std::mutex> lk(entry->mutex);
        entry->wait_list.push_back({txn_id, mode, lock_status::waiting});
    }
    std::unique_lock<std::mutex> lk(entry->mutex);
    entry->cond_var.wait(lk, [&]{ 
        
        if (mode == lock_mode::exclusive) {
            auto head = entry->wait_list.begin();
            if (std::get<0>(*head) == txn_id) {
                *head = {txn_id, mode, lock_status::granted};
                entry->current_lock_mode = lock_mode::exclusive;
                return true;
            }
            else
                return false;    
        }
        else {
            auto iter = entry->wait_list.begin();
            bool all_shared = true;
            bool all_granted = true;
            for(; iter != entry->wait_list.end(); iter++)
            {
                if (std::get<0>(*iter) == txn_id) {
                    if (all_shared && all_granted) {
                        *iter = {txn_id, mode, lock_status::granted};
                        entry->shared_lock_cnt++;
                    }
                    return all_shared && all_granted;
                }
                else {
                    if (std::get<1>(*iter) != lock_mode::shared)
                        all_shared = false;
                    
                    if (std::get<2>(*iter) != lock_status::granted)
                        all_granted = false;
                }
            }

        }
    });
    
    if (mode == lock_mode::shared)
        entry->cond_var.notify_all();
}

/*
When the lock manager receives an unlock message from a transaction, it
deletes the record for that data item in the linked list corresponding to that
transtion.

It tests the record that follows, if any, as described in the previous
paragraph, to see if that request can now be granted. If it can, the lock manager
grants that request, and process the record following it, if any, similarly, 
and so on.
*/
void lock_manager::unlock(txn_id_t txn_id, data_item item)
{
    lock_entry* entry = lock_table_->search(item);
    bool need_notify = false;
    {
        std::lock_guard<std::mutex> lk(entry->mutex);
        entry->wait_list.remove({txn_id, entry->current_lock_mode, lock_status::granted});
        if ( (entry->current_lock_mode == lock_mode::shared && --entry->shared_lock_cnt == 0) 
            || entry->current_lock_mode == lock_mode::exclusive) {
            entry->current_lock_mode = lock_mode::none;
            need_notify = true;
        }
    }
    
    if (need_notify)
        entry->cond_var.notify_all();

    if (entry->wait_list.size() == 0)
        lock_table_->remove(item);
}