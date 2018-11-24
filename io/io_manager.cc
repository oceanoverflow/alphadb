#include "io_manager.h"
#include "buffer_manager.h"
#include "file_handle.h"
#include "exceptions.h"

#include "unistd.h"
#include <fcntl.h>

io_manager::io_manager()
{
    buffer_manager_ = new buffer_manager(POOL_SIZE, BUFFER_SIZE);
}

io_manager::~io_manager()
{
    delete buffer_manager_;
}

void io_manager::create_file(const char *filename)
{
    int fd;
    int num_bytes;

    if ((fd = open(filename, O_CREAT | O_EXCL | O_WRONLY, CREATION_MASK)) < 0) {
        throw std::exception{};//throw unix_error{};
    }
    
    char buf[BUFFER_SIZE];
    memset(buf, 0, BUFFER_SIZE);
    file_header* header = reinterpret_cast<file_header *>(buf);
    header->page_num = 0;
    header->first_free = -1;

    
    if ((num_bytes = write(fd, buf, BUFFER_SIZE)) != BUFFER_SIZE) {
        close(fd);
        unlink(filename);
 
        if (num_bytes < 0) {
            throw std::exception{};//throw unix_error{};
        }
        else {
            throw std::exception{};//throw incomplete_header_write_error{};
        }
                
    }
    
    if (close(fd) < 0) {
        throw std::exception{};//throw unix_error{};
    }
    
}

file_handle io_manager::open_file(const char *filename)
{
    file_handle handle;
    
    if ((handle.fd_ = open(filename, O_RDWR)) < 0) {
        // throw unix_error{};
        throw std::exception{};
    }

    int num_bytes = read(handle.fd_, (void *)&(handle.header_), sizeof(file_header));

    if (num_bytes != sizeof(file_header)) {
        throw std::exception{};//num_bytes < 0 ? throw unix_error{} : throw incomplete_header_read_error{};
    }

    handle.file_is_open_ = true;
    handle.header_changed_ = false;
    handle.buffer_manager_ = buffer_manager_;
    return handle;
}

void io_manager::close_file(file_handle& handle)
{
    if (!handle.file_is_open_) {
        throw std::exception{};//throw closed_file_exception{};
    }
    handle.flush_pages();

    if (close(handle.fd_) < 0) {
        throw std::exception{};//throw unix_error{};
    }
    
    handle.file_is_open_ = false;
    handle.buffer_manager_ = nullptr;
}

void io_manager::destroy_file(const char *filename)
{
    
    if (unlink(filename) < 0) {
        throw std::exception{};//throw unix_error{};
    }

}

std::ostream& operator<<(std::ostream& os, const io_manager& mgr)
{
    os << *(mgr.buffer_manager_) << std::endl;
    return os;
}