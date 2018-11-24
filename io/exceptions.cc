#include "exceptions.h"
#include <sstream>
#include <string>

closed_file_exception::closed_file_exception()
{
}

const char* closed_file_exception::what() const noexcept
{
    std::ostringstream ss;

    ss << "invalid file descriptor (file closed)";

    static std::string message;
    message = ss.str();
    return message.c_str();
}

no_mem_error::no_mem_error() 
{
}

const char* no_mem_error::what() const noexcept
{
    std::ostringstream ss;

    ss << "Not enough memory";

    static std::string message;
    message = ss.str();
    return message.c_str();
}

no_buf_error::no_buf_error()
{
}

const char* no_buf_error::what() const noexcept
{
    std::ostringstream ss;

    ss << "Not enough buffer space";
    
    static std::string message;
    message = ss.str();
    return message.c_str();
}