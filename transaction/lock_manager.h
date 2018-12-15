#pragma once

#include "lock_table.h"

/*
A lock manager can be implemented as a process that receives messages from
transactions and sends message in reply. The lock-manager process replies to
lock-request messages with lock_grant messages, or with messages requesting
rollback of transaction (in case of deadlocks). Unlock messages require only
an acknowledgement in response, but may result in a grant message to another
waiting transaction.

The lock manager uses this data structure: For each data item that is currently
locked, it maintains a linked list of records, one for each request, in the order in
which the requests arrived. It uses a hash table, indexed on the name of a data item,
to find the linked list (if any) for a data item; this table is called the lock
table. Each record of the linked list for a data item notes which transaction made
the request, and what lock mode it requested. The record also notes if the request
has currently been granted.

This algorithm guarantess freedom from starvation for lock requests, since a request
can never be granted while a request received earlier is waiting to be granted.
*/

class lock_manager
{
private:
    lock_table* lock_table_;
public:
    lock_manager();
    ~lock_manager();

    void lock_s(txn_id_t txn_id, data_item item) { lock(txn_id, lock_mode::shared, item); }
    void lock_x(txn_id_t txn_id, data_item item) { lock(txn_id, lock_mode::exclusive, item); }
    
    void lock(txn_id_t txn_id, lock_mode mode, data_item item);
    void unlock(txn_id_t txn_id, data_item item);
};