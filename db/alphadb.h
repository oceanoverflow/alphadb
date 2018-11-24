#pragma once

#include <map>
#include <string>
#include "sql.h"

#include "io_manager.h"
#include "txn_manager.h"
#include "index_manager.h"
#include "table_manager.h"

const std::string DB_BASE_DIR{"~/"};

class alphadb
{
private:
    io_manager*    io_mgr_;
    txn_manager*   txn_mgr_;
    index_manager* idx_mgr_;
    table_manager* tbl_mgr_;

    std::string current_dir_;
    std::map<std::string, bool> db_opened_;
    std::map<std::string, std::string> metadata_;

    std::map<std::string, table_handle> tables_;
    std::map<std::pair<std::string, int>, index_handle> indeces_;
public:
    alphadb();
    ~alphadb();
    bool serve_command(const std::string& command, std::string& reply);
private:
    bool create_db(const std::string& name);     // TODO
    bool open_db(const std::string& name);       // DONE
    bool close_db(const std::string& name);      // TODO 
    bool drop_db(const std::string& name);       // DONE

    // CREATE TABLE students (name STRING, student_number INTEGER, city STRING, grade DOUBLE);
    bool create_table(const std::string& name);  // TODO 
    // DROP TABLE students; 
    bool drop_table(const std::string& name);    // TODO
    // SHOW TABLES;
    bool show_tables();                          // TODO

    bool create_index(const std::string& name);  // TODO
    // INSERT INTO students VALUES ("MAX", 11223344, 'wenzhou', 5.0);
    // SELECT * FROM students WHERE name = "MAX" ;
    // UPDATE students SET city = 'hangzhou' WHERE name = 'MAX';
    // DELETE FROM students WHERE grade < 3.0;
    bool drop_index(const std::string& name);    // TODO
};
