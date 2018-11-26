#include "table_manager.h"
#include <string.h>

table_manager::table_manager(io_manager& io_mgr): io_mgr_{io_mgr} {}

table_manager::~table_manager() {}

void table_manager::create_table(const char* filename, const schema& s)
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

    write_schema(data + sizeof(table_header), s);

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

    table_handle th = table_handle(fh, header);
    
    read_schema(data + sizeof(table_header), th.schema_);

    fh.unpin_page(0);

    return th;
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

void table_manager::read_schema(const void* src, schema& s)
{
    int offset = 0;
    int size;
    memcpy(&size, src, sizeof(size));
    offset += sizeof(size);

    for(int i = 0; i < size; i++)
    {
        column_definition def;
        int len;
        memcpy(&len, src + offset, sizeof(len));
        offset += sizeof(len);

        char* name = new char[len];
        memcpy(name, src + offset, len);
        def.name = name;
        offset += len;

        // TODO enum class
        memcpy(&def.type.type, src + offset, sizeof(static_cast<std::underlying_type<column_type>::type>(def.type.type)));
        offset += sizeof(static_cast<std::underlying_type<column_type>::type>(def.type.type));

        memcpy(&def.type.length, src + offset, sizeof(def.type.length));
        offset += sizeof(def.type.length);

        memcpy(&def.nullable, src + offset, sizeof(bool));
        offset += sizeof(bool);

        s.col_defs_.push_back(def);
        s.col_name_to_id_[name] = i;
    }
}

void table_manager::write_schema(void* dst, const schema& s)
{
    int offset = 0;
    int size = s.col_defs_.size();
    memcpy(dst, &size, sizeof(size));

    for(int i = 0; i < size; i++)
    {
        column_definition def = s.col_defs_[i];
        int namelen = strlen(def.name);
        memcpy(dst + offset, &namelen, sizeof(namelen));
        offset += sizeof(namelen);

        memcpy(dst + offset, def.name, namelen);
        offset += namelen;

        // TODO enum class
        memcpy(dst + offset, &static_cast<std::underlying_type<column_type>::type>(def.type.type), sizeof(static_cast<std::underlying_type<column_type>::type>(def.type.type)));
        offset += sizeof(static_cast<std::underlying_type<data_type>::type>(def.type.type));

        memcpy(dst + offset, &def.type.length, sizeof(def.type.length));
        offset += sizeof(def.type.length);

        memcpy(dst + offset, &def.nullable, sizeof(bool));
        offset += sizeof(bool);
    }
}