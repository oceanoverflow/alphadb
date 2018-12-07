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
void lock_manager::lock(txn_id_t id, data_item item)
{
    lock_entry* entry = lock_table_->add(item);
    std::unique_lock<std::mutex> lk(entry->mutex);
    entry->cnt++;
    entry->cond_var.wait(lk, [&]{ return !entry->locked; });
    entry->txn_id = id;
    entry->locked = true;
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
void lock_manager::unlock(txn_id_t id, data_item item)
{
    lock_entry* entry = lock_table_->search(item);
    entry->locked = false;
    entry->txn_id = 0;
    entry->cnt--;
    entry->cond_var.notify_one();
    if (entry->cnt == 0) {
        lock_table_->remove(item);
    }
}