#pragma once

#include "txn.h"

/*
two-phase locking: All locks are acquired in a transaction before any lock is released.

strict
two-phase locking: The transaction holds all locks until completion.
*/

class twopl_txn: public txn
{
private:
    
public:
    twopl_txn();
    ~twopl_txn();

    int rtti() const override { return 1; }

    void abort() override;
    bool commit() override;
};