#include "table_manager.h"

table_manager::table_manager(io_manager& io_mgr): io_mgr_{io_mgr} {}

table_manager::~table_manager() {}

void table_manager::create_table(const char* filename)
{
    io_mgr_.create_file(filename);

    file_handle fh = io_mgr_.open_file(filename);

    page_handle header_page = fh.allocate_page();
    char* data = header_page.data();
    int page = header_page.page();

    table_header header;
    header.first_free = -1;
    header.num_pages = 1;

    memcpy(data, &header, sizeof(table_header));

    fh.mark_dirty(page);
    fh.unpin_page(page);

    io_mgr_.close_file(fh);
}

table_handle table_manager::open_table(const char* filename)
{
    file_handle fh = io_mgr_.open_file(filename);

    page_handle ph = fh.get_this_page(0);
    char* data = ph.data();

    table_header header;
    memcpy(&header, data, sizeof(table_header));

    table_handle rfh = table_handle(fh, header);

    fh.unpin_page(0);

    return rfh;
}

void table_manager::close_table(table_handle& th)
{  
    file_handle fh = th.file_handle_;
    page_handle ph = fh.get_this_page(0);
    char* data = ph.data();
    memcpy((void *)data, reinterpret_cast<const void *>(&th.table_header_), sizeof(th.table_header_));
    fh.mark_dirty(0);
    fh.unpin_page(0);
    fh.force_page();
}

void table_manager::destroy_table(const char* filename)
{
    io_mgr_.destroy_file(filename);
}