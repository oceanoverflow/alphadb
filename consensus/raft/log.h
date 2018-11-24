#pragma once

#include <string>
#include <ostream>

struct log
{   
    uint64_t index;
    uint64_t term;
};

struct task_log: log
{
    std::string sql;
}
