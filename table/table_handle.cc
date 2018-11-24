#include "table_handle.h"

#include <memory>
#include <utility>

table_handle::table_handle(file_handle& fh, table_header header): file_handle_{fh}, table_header_{header} 
{
    
}

table_handle::~table_handle() {}

table_handle::table_handle(const table_handle& handle): file_handle_{handle.file_handle_}
{
    this->header_changed_ = handle.header_changed_;
    this->file_is_open_ = handle.file_is_open_;
    this->table_header_ = handle.table_header_;
}

table_handle& table_handle::operator=(const table_handle& handle)
{
    if (this != &handle) {
        this->header_changed_ = handle.header_changed_;
        this->file_is_open_ = handle.file_is_open_;
        this->file_handle_ = handle.file_handle_;
        this->table_header_ = handle.table_header_;
    }
    return *this;
}

record table_handle::get_record(const rid& id) const
{
    page_handle ph = file_handle_.get_this_page(id.page());
    auto format = std::make_unique<table_format>(ph);
    tuple_t tuple;
    format->get(slot_ptr_t(id.slot()), tuple);
    record r;
    r.set(static_cast<char *>(tuple.first), tuple.second, id);
    file_handle_.unpin_page(ph.page());
    return r;
}

rid table_handle::insert_record(const char* data, int len)
{
    int nextfree = get_next_free(len);
    page_handle ph = file_handle_.get_this_page(nextfree);
    auto format = std::make_unique<table_format>(ph);
    tuple_t tuple;
    tuple = std::make_pair((void *)data, len);
    int slot = format->put(tuple);
    file_handle_.unpin_page(ph.page());
    return rid{ph.page(), slot};
}

bool table_handle::delete_record(const rid& id)
{
    page_handle ph = file_handle_.get_this_page(id.page());
    auto format = std::make_unique<table_format>(ph);
    bool ret = format->remove(slot_ptr_t(id.slot()));
    file_handle_.unpin_page(ph.page());
    return ret;
}

bool table_handle::update_record(const record& r)
{
    if (!delete_record(r.get_rid()))
        return false;
    insert_record(r.data(), r.size());
}

int table_handle::get_next_free(int need) const
{
    // maximum 2 seek
    int seek = 0;
    int e = table_header_.first_free;
    
    while(e > 0){
        if (seek > 2) break;
        page_handle ph = file_handle_.get_this_page(e);
        auto format = std::make_unique<table_format>(ph);
        int space = format->hdr.free_upper - format->hdr.free_lower;
        if (space > need) return e;
        e = format->hdr.next_free;
        seek++;
    }

    page_handle ph = file_handle_.allocate_page();
    // table_header_.first_free = ph.page();
    return ph.page();
}