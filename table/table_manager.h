#pragma once

#include "schema.h"
#include "io_manager.h"
#include "table_handle.h"

class table_manager
{
private:
    io_manager& io_mgr_;
public:
    table_manager(io_manager& io_mgr);
    ~table_manager();

    void create_table(const char* filename, const schema& s);
    table_handle open_table(const char* filename);
    void close_table(table_handle& rfh);
    void destroy_table(const char* filenam);
private:
    void read_schema(const void* src, schema& s);
    void write_schema(void* dst, const schema& s);
};