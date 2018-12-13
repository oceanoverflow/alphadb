#pragma once

#include "txn.h"

/*
two-phase locking: All locks are acquired in a transaction before any lock is released.

strict
two-phase locking: The transaction holds all locks until completion.

      +-- 2PL with deadlock detection.
     /
2PL ----- 2PL with non-waiting deadlock prevention.
     \   
      +-- 2PL with wait-and-die deadlock prevention.
*/

class twopl_txn: public txn
{
private:
    
public:
    twopl_txn();
    ~twopl_txn();

    void abort() override;
    bool commit() override;
};