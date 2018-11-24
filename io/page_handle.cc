#include "page_handle.h"

page_handle::page_handle(): page_{-1}, data_{nullptr}
{
}

page_handle::~page_handle()
{
}

page_handle::page_handle(const page_handle& handle): page_{handle.page_}, data_{handle.data_}
{
}

page_handle& page_handle::operator=(const page_handle& handle)
{
    
    if (this != &handle) {
        this->page_ = handle.page_;
        this->data_ = handle.data_;
    }

    return *this;
    
}