#pragma once

#include "txn.h"

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
