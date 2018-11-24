#pragma once

#include "rid.h"
#include "record.h"
#include "file_handle.h"
#include "table_format.h"

struct table_header
{
    int first_free;
    int first_page;
    int num_pages;
};

class table_handle
{
    friend class table_manager;
    friend class table_iterator;
private:
    bool header_changed_;
    bool file_is_open_;
    file_handle& file_handle_;
    table_header table_header_;
public:
    table_handle();
    table_handle(file_handle& fh, table_header header);
    ~table_handle();
    table_handle(const table_handle& handle);
    table_handle& operator=(const table_handle& handle);

    record get_record(const rid& id) const;       // DONE
    rid insert_record(const char* data, int len); // TODO
    bool delete_record(const rid& id);            // DONE
    bool update_record(const record& r);          // DONE 

    int get_next_free(int need) const; // get free page
private:
    void pin_page(int page);
    void unpin_page(int page);
    
};