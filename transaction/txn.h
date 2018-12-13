#pragma once

#include "iterator.h"
#include "table_handle.h"
#include "index_handle.h"

#include <vector>

typedef uint64_t txn_id_t;

class txn_manager;

/*
txn: a logical unit of work that takes the db from one consistent state to another,
txns can terminate successfully and commit OR terminate unsuccessfully and be aborted
aborted txns must be undone (ROLLBACK) if they changed the db, COMMITted txns cannot 
be rolled back.

                          +------------------+    +------------------+
                     +--->|partially commited|--->|     commited     |---+
                    /     +------------------+    +------------------+    \
                   /                |                                      \
       +----------+                 |                                       +-------+
------>|active txn|                 | abort                                 |  end  |
       +----------+                 |                                       +-------+
                   \                v                                      /
                    \     +------------------+    +------------------+    /
              abort  +--->|      failed      |--->|      aborted     |---+
                          +------------------+    +------------------+

*/

class txn
{
protected:
    const txn_manager* txn_mgr_;
    txn_id_t txn_id_;
    txn(const txn_manager* mgr, txn_id_t txn_id);
public:
    ~txn();

    virtual void abort() = 0;
    virtual bool commit() = 0;

    bool commit_or_abort();

    virtual bool insert_record(const table_handle& handle, const std::vector<char *>& records) = 0;
    virtual bool delete_record(const table_handle& handle, const rid& id) = 0;

    std::vector<rid> query(const index_handle& handle, const criterion& cri, bool desc = false);

    table_handle get_table(const std::string& name);
    index_handle get_index(const std::string& name, int index);
};

class unsafe_txn: public txn
{
private:
    
public:
    unsafe_txn();
    ~unsafe_txn();

    void abort() override {}
    bool commit() override { return true; }

    virtual bool read_column(const table_handle& handle, const rid& id, column_id_t col_id, void* value) = 0;
    virtual bool write_column(const table_handle& handle, const rid& id, column_id_t col_id, const char*& value) = 0;
    virtual bool insert_record(const table_handle& handle, const std::vector<char *>& records) = 0;
    virtual bool delete_record(const table_handle& handle, const rid& id) = 0;

    std::vector<rid> query(const index_handle& handle, const criterion& cri, bool desc = false);
};