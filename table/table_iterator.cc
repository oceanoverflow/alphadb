#include "table_iterator.h"
#include "table_format.h"
#include <string>
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

    tuple_t t;
    format->get(now_.slot(), t);
    
    while(!fit(t, criterions_)){
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

bool table_iterator::fit(tuple_t t, const std::vector<criterion *>& cs) const
{
    void* data = t.first;
    ssize_t len = t.second;
    bool ret;
    
    for(int i = 0; i < cs.size(); i++)
    {
        auto cri = cs[i];
        char* id = cri->column.name;
        // column_id_t idx = tbl_handle_.schema_.col_name_to_id_[std::string(id)];
        
        int offset = 0;
        
        for(auto&& c : tbl_handle_.schema_.col_defs_)
        {
            // determine if equals, if equals then break
            // if (equals) break;
            offset += c.type.length;
        }
        
        // type conversion

        if (ret == false) break;
    }
    
    return ret;
}