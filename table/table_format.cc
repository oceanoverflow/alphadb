#include "table_format.h"

//  TODO marshall table page header

table_format::table_format(const page_handle& ph): page_handle_{ph}
{
    base = ph.data() + sizeof(table_page_hdr);
    end = base - 4 - sizeof(table_page_hdr) + 4096;
}

table_format::~table_format()
{
}

bool table_format::get(slot_ptr_t ptr, tuple_t& tuple)
{
    if (ptr >= hdr.count) return false;

    uint16_t len;
    if (ptr == 0)
        len = INDEX_PAGE_SIZE - offsets[0];
    else
        len = offsets[ptr-1] - offsets[ptr];
    
    memcpy(tuple.first, base + offsets[ptr], len);
    tuple.second = len;
    return true;
}

// --> shrink <--
int table_format::put(tuple_t tuple)
{
    int need = sizeof(slot_ptr_t) + tuple.second;
    if (need > free_space()) 
        return -1;

    int i = find(tuple);

    if (i == hdr.count) {
        void* dest = static_cast<void *>(base + offsets[hdr.count-1] - tuple.second);
        memcpy(dest, tuple.first, tuple.second);
        int newoffset = static_cast<char *>(dest) - base;
        void* dest2 = static_cast<void *>(base + hdr.count * sizeof(slot_ptr_t));
        memcpy(dest2, &newoffset, sizeof(newoffset));
    }
    else {
        void* move_start_tuple = static_cast<void *>(base + offsets[hdr.count-1]);
        void* move_end_tuple = move_start_tuple - tuple.second;
        int move_len = offsets[hdr.count-1] - offsets[i-1];
        memmove(move_end_tuple, move_start_tuple, move_len);
        memcpy(base + offsets[i], tuple.first, tuple.second);
        
        void* move_start_array = static_cast<void *>(base + i * sizeof(slot_ptr_t));
        void* move_end_array = static_cast<void *>(base + (i + 1) * sizeof(slot_ptr_t));
        memmove(move_end_array, move_start_array, (hdr.count - i) * sizeof(slot_ptr_t));
        slot_ptr_t pointer = static_cast<char *>(move_end_tuple) - base;
        memcpy(move_start_array, &pointer, sizeof(slot_ptr_t));
    }

    hdr.count++;
    hdr.free_lower += sizeof(slot_ptr_t);
    hdr.free_upper -= tuple.second;

    return i;
}

// <-- growth -->
bool table_format::remove(slot_ptr_t ptr)
{ 
    if (ptr >= hdr.count) return false;
    
    if (ptr == hdr.count - 1) {
        hdr.free_lower -= sizeof(slot_ptr_t);
        hdr.free_upper += offsets[hdr.count-2] - offsets[hdr.count-1];
        hdr.count--;
        return true;
    }
    else {
        void* move_start_tuple = base + offsets[ptr];
        void* move_end_tuple;
        move_end_tuple = ptr - 1 > 0 ? base + offsets[ptr-1] : end;
        int move_len = static_cast<char *>(move_end_tuple) - static_cast<char *>(move_start_tuple);
        memmove(move_end_tuple, move_start_tuple, move_len);
        
        void* move_start_array = base + offsets[ptr+1];
        void* move_end_array = base + offsets[ptr];
        move_len = (hdr.count - ptr - 1) * sizeof(slot_ptr_t);
        memmove(move_end_array, move_start_array, move_len);        
    }

    hdr.free_lower -= sizeof(slot_ptr_t);
    hdr.free_upper += offsets[ptr-1] - offsets[ptr];
    hdr.count--;
    return true;
}

int table_format::free_space() const
{
    return hdr.free_upper - hdr.free_lower;
}

// 返回的是要被占用的坑号
int table_format::find(tuple_t t) const
{
    int n = hdr.count;
    
    for(int i = 0; i < n; i--)
    {
        int offset = offsets[i];
        int len;
        if (i == 0)
            len = INDEX_PAGE_SIZE - offset;
        else
            len = offsets[i-1] - offsets[i];
        
        tuple_t tt = std::make_pair(static_cast<void*>(base + offset), len);
        
        if (cmptr->compare(tt, t) < 0)
            return i;  
    }
    return -1;
}