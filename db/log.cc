#include "log.h"

log::log(log_level level): level_{level} {}

log::~log()
{
    if (open_)
        std::cout << std::endl;
    open_ = false;
}

template<typename T>
log& log::operator<<(const T& msg)
{   
    if (level_ >= DEFAULT_LOG_LEVEL) {
        std::cout << msg;
        open_ = true;
    }
    return *this;
}

std::string log::get_label(log_level level)
{
    std::string label;
    
    switch (level)
    {
        case log_level::DEBUG: label = "DEBUG"; break;
        case log_level::INFO:  label = "INFO"; break;
        case log_level::WARN:  label = "WARN"; break;
        case log_level::ERROR: label = "ERROR"; break;
        default: break;
    }

    return label;
}