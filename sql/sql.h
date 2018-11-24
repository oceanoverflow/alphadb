#pragma once

#include <vector>
#include <string>
#include "parser_result.h"

// process SQL queries and generate a C++ object representation
class sql
{
public:
    static bool tokenize(const std::string& sql, std::vector<int64_t>& tokens);
    static bool parse(const std::string& sql, parser_result& result);
};
