#include "record.h"
#include <string.h>

record::record(): size_{-1}, data_{nullptr}, rid_(-1, -1) {}

record::~record() { 
    if (data_ != nullptr) {
        delete[] data_;
    }
}

char* record::data() const
{
    return data_;
}

rid record::get_rid() const
{
    return rid_;
}

void record::set(char* data, int size, rid _rid)
{
    size_ = size;
    rid_ = _rid;
    
    if (data_ == nullptr)
        data_ = new char[size_];
    memcpy(data_, data, size_);
}
