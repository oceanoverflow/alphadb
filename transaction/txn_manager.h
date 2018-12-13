#pragma once

#include "txn.h"
#include "index_manager.h"
#include "table_manager.h"

#include <map>
#include <string>

/*
OLTP database systems support the part of an application that interacts
with end-users. End-users with the front-end application by sending it
requests to perform some function (e.g., reserve a seat on a flight).
The application processes these requests and then executes transactions
in the DBMS. Such users could be humans on their personal computer or
mobile device, or another computer program petentially running somewhere
else in the world.

A transaction in the context of one of these systems is the execution of
a sequence of one or more operation (e.g., SQL queries) on a shared database
to perform some higher-level function. It is the basic unit of change in a
DBMS: partial transactions are not allowed, and the effect of a group of
transactions on the database's state is equivalent to any serial execution
of all transactions. The transactions in modern OLTP workloads have three
salient characteristics: (1) they are short-lived (i.e. no user stalls),
(2) they touch a small subset of data using index look-ups (i.e., no full
table scans or large joins), and (3) they are repetitive (i.e., executing 
the same queries with different inputs).

An OLTP DBMS is expected to maintain four properties for each transaction
that it executes: (1) atomicity, (2) consistency, (3) isolation, and (4)
durability. These unifying concepts are collectively refered to with the
ACID acronym. Concurrency control permits end-users to access a database
in a multi-programmed fashion while preserving the illusion that each of
them is executing their transaction alone on a delicated system. It
essentially provides the atomicity and isolation guarantees in the system.
*/

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