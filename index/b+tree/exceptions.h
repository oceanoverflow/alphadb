#pragma once

#include <exception>

class leaf_not_found_exception : std::exception
{
public:
    explicit leaf_not_found_exception();
    const char* what() const noexcept override;
};

class node_not_found_exception : std::exception
{
public:
    explicit node_not_found_exception();
    const char* what() const noexcept override;
};

class record_not_found_exception : std::exception
{
public:
    explicit record_not_found_exception();
    const char* what() const noexcept override;
}