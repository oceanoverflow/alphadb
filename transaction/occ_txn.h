#pragma once

#include "twopl_txn.h"

/*
Optimistic Concurrency Control (OCC):
The DBMS tracks the read/write sets of each transaction and stores all of their
write operations in their private workspace. When a transaction commits, the 
system determines whether that transaction's read set overlaps with the write
set of any concurrent transactions. If no overlaps exists, then the DBMS applies
the changes from the transaction's workspace into the database; otherwise, the
transaction is aborted and restarted. The advantage of this approach for main
memory DBMSs is that transactions write their updates to shared memory only at
commit time, and thus the contention period is short.
*/

class occ_txn: public twopl_txn
{
private:
    /* data */
public:
    occ_txn(/* args */);
    ~occ_txn();
};