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
bool lock_manager::lock(txn_id_t id, data_item item, lock_mode mode)
{
    return lock_table_->add(item, id, mode);
}

/*
When the lock manager receives an unlock message from a transaction, it
deletes the record for that data item in the linked list corresponding to that
transtion.

TODO: It tests the record that follows, if any, as described in the previous
paragraph, to see if that request can now be granted. If it can, the lock manager
grants that request, and process the record following it, if any, similarly, 
and so on.
*/
bool lock_manager::unlock(txn_id_t id, data_item item)
{
    return lock_table_->remove(item, id);
}

/*
If a transaction aborts, the lock manager deletes any waiting request made
by the transaction. Once the database system has taken appropriate actions
to undo the transaction, it releases all locks held by the aborted transaction.
*/
void lock_manager::unlock_all(txn_id_t id)
{
    auto vec = lock_table_->m_[id];
    
    for(auto& item : *vec)
    {
        lock_table_->remove(item, id);
    }
}