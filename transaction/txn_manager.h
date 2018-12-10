#pragma once

#include "txn.h"
#include "index_manager.h"
#include "table_manager.h"

#include <map>
#include <string>

class txn_manager
{
private:
    table_manager& tbl_mgr_;
    index_manager& idx_mgr_;

    std::map<std::string, table_handle> tables_;
    std::map<std::string, index_handle> indexes_;
public:
    txn_manager(index_manager& idx_mgr, table_manager& tbl_mgr);
    virtual ~txn_manager();
    txn_manager(const txn_manager& mgr) = delete;
    txn_manager& operator=(const txn_manager& mgr) = delete;

    virtual txn* start(txn_id_t id) = 0;
    
    table_handle get_table(const std::string& name);
    index_handle get_index(const std::string& name, int index);
};