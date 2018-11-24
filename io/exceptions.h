#pragma once

#include <exception>

class closed_file_exception: public std::exception
{
public:
    explicit closed_file_exception();
    const char* what() const noexcept override;
};

class eof_exception: public std::exception
{
public:
    explicit eof_exception();
    const char* what() const noexcept override;
};

class page_already_free_exception: public std::exception
{
public:
    explicit page_already_free_exception();
    const char* what() const noexcept override;
};

class invalid_page_exception: public std::exception
{
public:
    explicit invalid_page_exception();
    const char* what() const noexcept override;
};

class page_pinned_exception: public std::exception
{
public:
    explicit page_pinned_exception();
    const char* what() const noexcept override;
};

class page_already_unpinned_exception: public std::exception
{
public:
    explicit page_already_unpinned_exception();
    const char* what() const noexcept override;
};

class page_not_in_buf_exception: public std::exception
{
public:
    explicit page_not_in_buf_exception();
    const char* what() const noexcept override;
};

class no_mem_error: public std::exception
{
public:
    explicit no_mem_error();
    const char* what() const noexcept override;
};

class no_buf_error: public std::exception
{
public:
    explicit no_buf_error();
    const char* what() const noexcept override;
};

class incomplete_read_error: public std::exception
{
public:
    explicit incomplete_read_error();
    const char* what() const noexcept override;
private:
    int read_num_;
};

class incomplete_header_read_error: public std::exception
{
public:
    explicit incomplete_header_read_error();
    const char* what() const noexcept override;
private:
    int read_num_;
};

class incomplete_write_error: public std::exception
{
public:
    explicit incomplete_write_error();
    const char* what() const noexcept override;
private:
    int write_num_;
};

class incomplete_header_write_error: public std::exception
{
public:
    explicit incomplete_header_write_error();
    const char* what() const noexcept override;
private:
    int write_num_;
};

class unix_error: public std::exception
{
public:
    explicit unix_error();
    const char* what() const noexcept override;
};




