#pragma once

#include "rid.h"
#include "comparator.h"
#include "page_handle.h"
#include <string.h>
#include <utility>

// variable length

/*   page_id 
*      |                          +----- slot_id
*      |            base --+      |    
*      v                   v      v               4092B
*      +----+--------------+------+------+------+------+
*      | Nt |table_page_hdr| ptr0 | ptr1 | ptr2 | ptr3 |
*      +----+-+--------+---+--+---+------+------+------+
*      | ptr4 | .....  | ptrX |<|                      |
*      +---+--+--------+---+--+                        |
*      |   |               |                           |
*      |   |  +------------+                           |
*      |   |  |                                        |
*      |   +--+--------------+                         |
*      |      |              |                  |      |
*      |      v              v                  v      |
*      |      +--------+-----+------------------+------+
*      |    |>|(tupleX)| ... |>    (tuple4)     |tuple3|
*      +------+-+------+-----+---------+--------+------+
*      |(tuple2)|>     (tuple1)        |>   (tuple0)   |  
*      +--------+----------------------+---------------+
*                           largest  <--------  smallest
*                       tuple_len[i] = ptr[i] - ptr[i-1]
*            for example: tuple_len[4] = ptr[3] - ptr[4]
*  special case: tuple_len[0] = INDEX_PAGE_SIZE - ptr[0]
*/

constexpr const int INDEX_PAGE_SIZE{4096-sizeof(table_page_hdr)-4};

struct table_page_hdr
{
    int next_free;
    int count;
    int free_lower;
    int free_upper;
};

typedef uint16_t slot_ptr_t;
typedef std::pair<void *, ssize_t> tuple_t;  

struct table_format
{
    table_page_hdr hdr;
    char*       base;
    char*       end;
    slot_ptr_t* offsets;
    page_handle page_handle_;

    comparator* cmptr;

    table_format(const page_handle& ph);
    ~table_format();
    table_format& operator=(const table_format& format);
    table_format(const table_format& format);

    int put(tuple_t tuple);
    bool get(slot_ptr_t ptr, tuple_t& tuple);
    bool remove(slot_ptr_t ptr);
private:
    int free_space() const;
    int find(tuple_t t) const;
};