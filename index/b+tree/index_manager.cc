#include "index_manager.h"
#include "rid.h"
#include <string.h>
#include <sstream>

index_manager::index_manager(io_manager& io_mgr): io_mgr_{io_mgr} {}

index_manager::~index_manager() {}

void index_manager::create_index(const char* filename, int index_no, int attr_type, int attr_len)
{
    std::ostringstream oss;
    oss << filename << "." << index_no;

    io_mgr_.create_file(oss.str().c_str());

    file_handle fh = io_mgr_.open_file(oss.str().c_str());
    page_handle ph = fh.allocate_page();
    
    char* page_data = ph.data();

    index_header header;
    header.root_page = -1;
    header.pair_size = attr_len + sizeof(rid);
    header.height = 0;
    header.attr_type = attr_type;
    header.attr_length = attr_len;

    int fanout = (DEFAULT_PAGE_SIZE - sizeof(page_header)) / (sizeof(rid) + attr_len);
    while( fanout * (sizeof(rid) + attr_len) > (DEFAULT_PAGE_SIZE - sizeof(page_header)) ){
        fanout--;
    }
    header.fanout = fanout;
    
    memcpy(page_data, &header, sizeof(index_header));

    int page_num = ph.page();
    assert(page_num == 0);

    fh.mark_dirty(0);
    fh.unpin_page(0);

    io_mgr_.close_file(fh);
}

void index_manager::destroy_index(const char* filename, int index_no)
{
    std::ostringstream oss;
    oss << filename << "." << index_no;
    io_mgr_.destroy_file(oss.str().c_str());
}

index_handle index_manager::open_index(const char* filename, int index_no)
{
    std::ostringstream oss;
    oss << filename << "." << index_no;
    file_handle fh = io_mgr_.open_file(oss.str().c_str());

    page_handle ph = fh.get_this_page(0);
    char* data = ph.data();

    index_header header;
    memcpy(&header, (const void*)data, sizeof(index_header));

    index_handle ih = index_handle(fh, header);

    fh.unpin_page(0);

    return ih;
}

void index_manager::close_index(index_handle& handle)
{
    if (handle.header_changed_) {
        
    }

    io_mgr_.close_file(handle.file_handle_);
}