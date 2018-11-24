#pragma once

#include <string>
#include <iostream>

// TODO
// print log time and log level

enum class log_level
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
};

const log_level DEFAULT_LOG_LEVEL{log_level::DEBUG};

class log
{
private:
    bool open_;
    log_level level_ = log_level::DEBUG;
public:
    log(log_level level);
    ~log();

    template<typename T>
    log& operator<<(const T& msg);
private:
    std::string get_label(log_level level);
};