#pragma once

#include "twopl_txn.h"

class occ_txn: public twopl_txn
{
private:
    /* data */
public:
    occ_txn(/* args */);
    ~occ_txn();
};