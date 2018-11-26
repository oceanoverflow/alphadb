#include "alphadb.h"

#include <vector>
#include <sstream>
#include <iostream>
#include <exception>

#include "parser_result.h"

#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>

#include <sys/stat.h>

alphadb::alphadb(): current_dir_{DB_BASE_DIR}
{ 
    io_mgr_ = new io_manager();
    tbl_mgr_ = new table_manager(*io_mgr_);
    idx_mgr_ = new index_manager(*io_mgr_);
    
    if (chdir(current_dir_.c_str()) < 0)
        throw std::exception{};
}

alphadb::~alphadb()
{
    delete idx_mgr_;
    delete tbl_mgr_;
    delete io_mgr_;
    delete txn_mgr_;
}

bool alphadb::create_db(const std::string& name)
{
    if (chdir(DB_BASE_DIR.c_str()) < 0 || mkdir(name.c_str(), 0777) < 0)
        return false;

    // create some meta data file using table_manager
    return true;    
}

bool alphadb::open_db(const std::string& name)
{   
    if (db_opened_[name])
        return true;

    current_dir_ += "/" + name;
    if (chdir(current_dir_.c_str()) < 0)
        return false;    
    
    db_opened_[name] = true;
    return true;
}

bool alphadb::close_db(const std::string& name)
{   
    if (!db_opened_[name])
        return true;

    current_dir_ = DB_BASE_DIR;
    
    if (chdir(current_dir_.c_str()) < 0)
        return false;
    
    db_opened_[name] = false;
    return true;
}

bool alphadb::drop_db(const std::string& name)
{
    if (db_opened_[name])
       close_db(name);

    std::stringstream ss;
    ss << "rm -r " << DB_BASE_DIR << "/" << name;
    
    if (system(ss.str().c_str()) != 0)
        return false;
    
    return true;       
}

bool alphadb::create_table(const std::string& name, const schema& s)
{
    tbl_mgr_->create_table(name.c_str(), s);
    return false;
}

bool alphadb::drop_table(const std::string& name)
{
    tbl_mgr_->destroy_table(name.c_str());
    return false;
}

bool alphadb::create_index(const std::string& name)
{
    idx_mgr_->create_index(name.c_str(), -1, -1, -1);
    return false;
}

bool alphadb::drop_index(const std::string& name)
{
    idx_mgr_->destroy_index(name.c_str(), -1);
    return false;
}

bool alphadb::serve_command(const std::string& command, std::string& reply)
{
    parser_result result;
    parser::parse(command, result);
    // switch on the result and do the respective command
    auto stmts = result.get_statements();
    
    for(auto&& stmt : stmts)
    {
        switch (stmt->type())
        {
            case statement_type::CREATE:
                create_statement* cstmt = dynamic_cast<create_statement *>(stmt);
                create_db(cstmt->table_name);
                break;

            case statement_type::INSERT:
                insert_statement* istmt = dynamic_cast<insert_statement *>(stmt);
                
                if (tables_.find(istmt->table_name) == tables_.end()) {
                    table_handle th = tbl_mgr_->open_table(istmt->table_name);
                    tables_.insert({std::string(istmt->table_name), th});
                    th.insert_record("TODO", 1);
                }
                else {
                    table_handle th = tables_[std::string(istmt->table_name)];
                    th.insert_record("TODO", 1);
                } 
                // TODO index
                break;

            case statement_type::SELECT:
                select_statement* sstmt = dynamic_cast<select_statement *>(stmt);
                break;

            case statement_type::UPDATE:
                update_statement* ustmt = dynamic_cast<update_statement *>(stmt);
                // table
                
                if (tables_.find(ustmt->table_name) == tables_.end()) {
                    // open the table and put the file handle in the map
                    
                }
                else {
                    table_handle th = tables_[ustmt->table_name];
                    // th.update_record();
                }
                
                // index
                break;

            case statement_type::DELETE:
                delete_statement* dstmt = dynamic_cast<delete_statement *>(stmt);
                if (tables_.find(istmt->table_name) != tables_.end()) {
                    table_handle th = tables_[dstmt->table_name];
                    th.delete_record(rid{-1, -1});
                }
                
                // find the record according to index, then delete it
                break;

            case statement_type::DROP:
                drop_statement* dstmt = dynamic_cast<drop_statement *>(stmt);
                drop_table(dstmt->table_name);
                drop_index(dstmt->table_name);
                break;

            case statement_type::SHOW:
                show_statement* sstmt = dynamic_cast<show_statement *>(stmt);
                break;

            default:
                break;
        }
    }
    
    return false;
}