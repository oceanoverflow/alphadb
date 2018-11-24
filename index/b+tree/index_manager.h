#pragma once

#include "io_manager.h"
#include "index_handle.h"

class index_manager
{
private:
    io_manager& io_mgr_;
public:
    index_manager(io_manager& io_mgr);
    ~index_manager();

    friend std::ostream& operator<<(std::ostream& os, const index_manager& tree);

    void create_index(const char* filename, int index_no, int attr_type, int attr_len);
    void destroy_index(const char* filename, int index_no);
    index_handle open_index(const char* filename, int index_no);
    void close_index(index_handle& handle);
};

