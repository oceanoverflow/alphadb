#include "file_handle.h"
#include "exceptions.h"
#include "unistd.h"

file_handle::file_handle(): file_is_open_{false}, buffer_manager_{nullptr} {}

file_handle::~file_handle() {}

file_handle::file_handle(const file_handle& handle)
{
    this->fd_ = handle.fd_;
    this->header_ = handle.header_;
    this->file_is_open_ = handle.file_is_open_;
    this->header_changed_ = handle.header_changed_;
    this->buffer_manager_ = handle.buffer_manager_;
}

file_handle& file_handle::operator=(const file_handle& handle)
{  
    if (this != &handle) {
       this->fd_ = handle.fd_;
       this->header_ = handle.header_;
       this->file_is_open_ = handle.file_is_open_;
       this->header_changed_ = handle.header_changed_;
       this->buffer_manager_ = handle.buffer_manager_;
    }

    return *this;  
}

page_handle file_handle::allocate_page()
{
    int page;
    char* data;
    page_handle handle;

    if (file_is_open_ == false) {
        throw std::exception{};//throw closed_file_exception{};
    }

    if (header_.first_free != PAGE_LIST_END) {
        page = header_.first_free;
        data = buffer_manager_->get_page(fd_, page);
        header_.first_free = reinterpret_cast<page_header *>(data)->next_free;
    }
    else {
        page = header_.page_num;
        data = buffer_manager_->allocate_page(fd_, page);
        header_.page_num++;
    }

    header_changed_ = true;

    reinterpret_cast<page_header *>(data)->next_free = PAGE_USED;
    memset(data + sizeof(page_header), 0, DEFAULT_PAGE_SIZE - sizeof(page_header));

    mark_dirty(page);

    handle.page_ = page;
    handle.data_ = data + sizeof(page_header);

    return handle; 
}

void file_handle::dispose_page(int page)
{
    
    if (file_is_open_ == false) {
        throw std::exception{};//throw closed_file_exception{};
    }
    
    if (!is_valid_page(page)) {
        throw std::exception{};//throw invalid_page_exception{};
    }

    char* data = buffer_manager_->get_page(fd_, page, false);

    if (reinterpret_cast<page_header *>(data)->next_free != PAGE_USED) {
        unpin_page(page);
        throw std::exception{};//throw page_already_free_exception{};
    }

    reinterpret_cast<page_header *>(data)->next_free = header_.first_free;
    header_.first_free = page;
    header_changed_ = true;

    mark_dirty(page);
    unpin_page(page);

}

page_handle file_handle::get_this_page(int page) const
{
    page_handle handle;

    if (file_is_open_ == false) {
        throw std::exception{};//throw closed_file_exception{};
    }
    
    if (!is_valid_page(page)) {
        throw std::exception{};//throw invalid_page_exception{};
    }

    char* data = buffer_manager_->get_page(fd_, page);

    if ( reinterpret_cast<page_header *>(data)->next_free == -2 ) {
        handle.page_ = page;
        handle.data_ = data + sizeof(page_header);
        return handle;
    }
    
    unpin_page(page);
    throw std::exception{};//throw invalid_page_exception{};
    
}

void file_handle::flush_pages()
{
    
    if (file_is_open_ != true) {
        throw std::exception{};//throw closed_file_exception{};
    }
    
    if (header_changed_) {     
        
        if (lseek(fd_, 0, SEEK_SET) < 0) {
            throw std::exception{};//throw unix_error{};
        }

        int num_bytes = write(fd_, (const void *)&header_, sizeof(file_header));
        
        if (num_bytes < 0) {
            throw std::exception{};//throw unix_error{};
        }
        else if (num_bytes != sizeof(file_header)) {
            throw std::exception{};//throw incomplete_header_write_error{};
        }
        
        this->header_changed_ = false;
          
    }

    buffer_manager_->flush_pages(fd_);
       
}

void file_handle::unpin_page(int page) const
{
    if (file_is_open_ == false)
        throw std::exception{};//throw closed_file_exception{};

    if (!is_valid_page(page))
        throw std::exception{};//throw invalid_page_exception{};

    buffer_manager_->unpin_page(fd_, page);    
}

void file_handle::force_page(int page)
{
    if (file_is_open_ == false)
        throw std::exception{};//throw closed_file_exception{};
    
    if (header_changed_) { 
        if (lseek(fd_, 0, SEEK_SET) < 0)
            throw std::exception{};//throw unix_error{};

        int num_bytes = write(fd_, (const void *)&header_, sizeof(file_header));
        
        if (num_bytes < 0)
            throw std::exception{};//throw unix_error{};
        else if (num_bytes != sizeof(file_header))
            throw std::exception{};//throw incomplete_header_write_error{};
        
        this->header_changed_ = false;
    }
    
    buffer_manager_->force_page(fd_, page);
}

void file_handle::mark_dirty(int page) const
{
    if (file_is_open_ == false)
        throw std::exception{};//throw closed_file_exception{};
  
    if (!is_valid_page(page))
        throw std::exception{};//throw invalid_page_exception{};
    
    buffer_manager_->mark_dirty(fd_, page);
}

bool file_handle::is_valid_page(int page) const
{
    return page >= 0 && page < header_.page_num;
}

page_handle file_handle::get_next_page(int current) const
{
    if (file_is_open_ == false)
        throw std::exception{};

    if (current != -1 && !is_valid_page(current))
        throw std::exception{};//throw invalid_page_exception{};

    page_handle handle;

    for(current++; current < header_.page_num; current++)
    {
        try
        {
            handle = get_this_page(current);
            return handle;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    throw std::exception{};//throw eof_exception{};
    
}

page_handle file_handle::get_prev_page(int current) const
{
    if (file_is_open_ == false)
        throw std::exception{};//throw closed_file_exception{};

    if (current != header_.page_num && !is_valid_page(current))
        throw std::exception{};//throw invalid_page_exception{};

    page_handle handle;
    
    for(current--; current >= 0; current--)
    {
        
        try
        {
            handle = get_this_page(current);
            return handle;
        }
        catch(const std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        
    }

    throw std::exception{};//throw eof_exception{};  
}

page_handle file_handle::get_first_page() const
{
    return get_next_page(-1);
}

page_handle file_handle::get_last_page() const
{
    return get_prev_page(header_.page_num);
}

std::ostream& operator<<(std::ostream& os, const file_handle& handle)
{
    os << "header: " << handle.header_.page_num << std::endl;
    return os;
}