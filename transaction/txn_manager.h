#pragma once

#include "txn.h"

class txn_manager
{
private:
    
public:
    txn_manager();
    virtual ~txn_manager();

    virtual txn* start(txn_id_t id) = 0;
    
};
