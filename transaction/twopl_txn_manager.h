#pragma once

/*
two-phase locking (2PL) was the first provably correct method
of ensuring the correct execution of concurrent transactions
in a database system. Under this scheme, transactions have to
acquire locks for a particular element in the database before
they are allowed to execute a read or write operation on that
element. The transaction must acquire a read lock before it is
allowed to read that element, and similarly it must acquire a
write lock in order to modify that element. The DBMS maintains
locks for either each tuple or at a higher logical level (e.g.,
tables, partitions).

The ownership of locks is governed by two rules: (1) different
transactions connot simultaneously own conflicting locks, and
(2) once a transaction surrenders ownership of a lock, it may
never abtain additional locks, A read lock on an element conflicts
with a write lock on that same element. Likewise, a write lock
on an element conflicts with a write lock on the same element.

In the first phase of 2PL, known as the growing phase, the
transaction is allowed to acquire as many locks as it needs without
releasing locks. When the transaction releases a lock, it enters
the second phase, known as the shrinking phase; it is prohibited
from obtaining additional locks at this point. When the transaction
terminates (either by commiting or aborting), all the remaining
locks are automatically released back to the coordinator.

2PL is considered a pessimistic approach in that it assumes that
transactions will conflict and thus they need to acquire locks to
avoid this problem. If a transaction is unable to acquire a lock
for an element, then it is forced to wait until the lock becomes
available. If this waiting is uncontrolled (i.e. indefinite), then
the DBMS can incur deadlocks. Thus, a major difference among the
different variants of 2PL is in how they handle deadlocks and the
actions they take when a deadlock is detected.
*/

class twopl_txn_manager
{
private:
    /* data */
public:
    twopl_txn_manager(/* args */);
    ~twopl_txn_manager();
};


