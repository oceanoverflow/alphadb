#pragma once

#include "rid.h"
#include "schema.h"
#include "comparator.h"
#include <string.h>

// keys of identical length

/*                    
*                        +-- base
*                        v                      4096B
*   +----+---------------+------+------+------+------+
*   | Nt |index_page_hdr | RID0 | RID1 | RID2 | RID3 |
*   +----+---------------+------+------+------+------+
*   | RID4 | RID5 | RID6 | .... | RIDi <| || |> KEY0 |
*   +------+------+------+------+------+------+------+
*   | KEY1 | KEY2 | KEY3 | KEY4 | KEY5 | KEY6 | ...  |
*   +------+------+------+------+------+------+------+
*   | .... | KEYi |   Free Space                     |
*   +------+------+----------------------------------+
*   |                 Free Space                     |
*   +------------------------------------------------+
*   |                 Free Space                     |
*   +------------------------------------------------+
*   |                 Free Space                     |
*   +------------------------------------------------+
*   |                 Free Space                     |
*   +------------------------------------------------+
*
*  +--------------+--------------+--------------+--------------+-------------------------------------+-----------------------+
*  | (RID0, KEY0) | (RID1, KEY1) | (RID2, KEY2) | (RID3, KEY3) | ....................................| (RID[n-1], DUMMY_KEY) |
*  +--------------+--------------+--------------+--------------+-------------------------------------+-----------------------+
*
*      RIDi >= KEYi-1
*
*  For Internal Node: (sparse index)
*      KEY[n-1] is dummy key, RID[n-1] is a pointer to the next level node
*  For Leaf Node: (dense index)
*      KEY[n-1] is dummy key, RID[n-1] is a pointer to the next leaf node
*
*/

typedef void* key_t;

const int DUMMY_KEY{-1};
constexpr const int DEFAULT_KEY_SIZE{sizeof(int)};
constexpr const int INDEX_PAGE_SIZE{4096-sizeof(index_page_hdr)-4};

struct index_page_hdr
{
    int count;
    int fanout;
    int free_start;
    int free_space;
};

struct index_format
{
    char*  base;
    rid*   rids;
    key_t* keys;
    index_page_hdr hdr;

    comparator* cmptr;
    int key_len;
    data_type datatype;
    
    index_format();
    ~index_format();

    int find_in_leaf(key_t key) const;
    int find_in_internal(key_t key) const;
    
    bool search(key_t key, rid& id) const;
    bool insert(key_t key, const rid& id);
    bool remove(int index);
    bool remove(key_t key, const rid& id);
private:
    bool enough() const;
};