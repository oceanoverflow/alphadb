#pragma once

#include "table_handle.h"
#include "index_handle.h"

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

    virtual int rtti() const = 0;

    virtual void abort() = 0;
    virtual bool commit() = 0;

    bool commit_or_abort();

    virtual bool delete_record(const table_handle& handle) = 0;

    table_handle get_table(const std::string& name);
    index_handle get_index(const std::string& name, int index);

};


class unsafe_txn: public txn
{
private:
    
public:
    unsafe_txn();
    ~unsafe_txn();

    virtual int rtti() const { return 1; }

    void abort() override {}
    bool commit() override { return true; }
    
};

