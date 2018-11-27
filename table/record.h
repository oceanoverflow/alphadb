#pragma once

#include "rid.h"

#include <vector>
#include <iostream>

class record
{
private: 
    rid rid_;
    int size_;
    char* data_; 
public:
    record();
    ~record();

    rid get_rid() const;
    char* data() const;
    int size() const;
    void set(char* data, int size, rid _rid);

    friend std::ostream& operator<<(std::ostream& os, const rid& rid);
};