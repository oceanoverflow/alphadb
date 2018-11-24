#pragma once

#include <iostream>

class buffer_manager;
class file_handle;

const int CREATION_MASK{0600};
const int POOL_SIZE{2};
const int BUFFER_SIZE{4096};

class io_manager
{
private:
    buffer_manager* buffer_manager_;
public:
    io_manager();
    ~io_manager();
    void create_file(const char *filename);
    file_handle open_file(const char *filename);
    void close_file(file_handle& handle);
    void destroy_file(const char *file_handle);

    friend std::ostream& operator<<(std::ostream& os, const io_manager& mgr);
};
