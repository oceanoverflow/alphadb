#include "table_iterator.h"
#include "table_format.h"
#include <memory>

table_iterator::table_iterator(const table_handle& handle, const std::vector<criterion *>& cs): tbl_handle_{handle}, criterions_{cs}
{
    now_ = rid{tbl_handle_.table_header_.first_page, 0};
}

table_iterator::~table_iterator()
{
}

rid table_iterator::begin() const
{
    now_ = rid{-1, -1};
    return now_;
}

rid table_iterator::next() const
{
    page_handle ph = tbl_handle_.file_handle_.get_this_page(now_.page());
    auto format = std::make_unique<table_format>(ph);
    int cnt = format->hdr.count;
    
    while(!fit(now_, criterions_)){
        if (now_.slot() < cnt-1)
            now_ = rid{now_.page(), now_.slot() + 1};
        else {
            int next = format->hdr.next_free; // TODO ... next free -> next
            now_ = rid{next, 0};
        }
    }

    return now_; 
}

rid table_iterator::end() const
{
    return rid{-1, -1};
}

void table_iterator::reset()
{
    now_ = rid{-1, -1};
}

bool table_iterator::fit(rid id, const std::vector<criterion *>& cs) const
{
    return false;
}