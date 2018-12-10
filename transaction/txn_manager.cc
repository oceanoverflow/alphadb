#include "txn_manager.h"

txn_manager::txn_manager(index_manager& idx_mgr, table_manager& tbl_mgr): idx_mgr_{idx_mgr}, tbl_mgr_{tbl_mgr} {}

txn_manager::~txn_manager() {}

table_handle txn_manager::get_table(const std::string& name)
{
    auto it = tables_.find(name);
    if (it == tables_.end()) {
        table_handle handle = tbl_mgr_.open_table(name.c_str());
        tables_[name] = handle; 
        return handle;
    }
    else {
        return it->second;
    }
}

index_handle txn_manager::get_index(const std::string& name, int index)
{
    auto it = indexes_.find(name);
    if (it == indexes_.end()) {
        index_handle handle = idx_mgr_.open_index(name.c_str(), index);
        indexes_[name] = handle;
        return handle;
    }
    else {
        return it->second;
    }
}