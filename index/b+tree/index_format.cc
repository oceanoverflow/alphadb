#include "index_format.h"

index_format::index_format()
{
}

index_format::~index_format()
{
}

bool index_format::search(key_t key, rid& id) const
{
    for(int i = 0; i < hdr.count; i++) 
    {
        if (keys[i] == key) 
        {
            id = rids[i];
            return true;
        }   
    }     
    return false;  
}

bool index_format::insert(key_t key, const rid& id)
{
    /*
    if (K < L.K1)
        then insert P, K into L just before L.P1
    else begin
        Let Ki be the highest value in L that is less than K
        Insert P, K into L just after T.Ki
    end
    */
    int i = find_in_leaf(key);
    if (i != -1)
        return false; // TODO, now do not allow duplicate key, do it later
    
    int len = DEFAULT_KEY_SIZE * hdr.count + sizeof(rid) * (hdr.count - i);
    memmove(base + (i+1) * sizeof(rid), base + i * sizeof(rid), len);
    memcpy(base + i * sizeof(rid), &id, sizeof(id));

    keys = static_cast<key_t *>(static_cast<void *>(keys) + sizeof(rid));
    int len2 = DEFAULT_KEY_SIZE * (hdr.count - i);
    memmove(static_cast<void *>(keys) + len2, static_cast<void *>(keys) + len2 + DEFAULT_KEY_SIZE, len2);
    memcpy(static_cast<void *>(keys) + len2, key, DEFAULT_KEY_SIZE);

    hdr.count++;
    hdr.free_start += (sizeof(rid) + DEFAULT_KEY_SIZE);
    hdr.free_space -= (sizeof(rid) + DEFAULT_KEY_SIZE);
    return true;
}

bool index_format::remove(int index)
{
    if (index >= hdr.count)
        return false;
    
    int i = index;
    
    void* remove_key_end = base + sizeof(rid) * hdr.count + DEFAULT_KEY_SIZE * i;
    void* remove_key_begin = base + sizeof(rid) * hdr.count + DEFAULT_KEY_SIZE * (i + 1);
    memmove(remove_key_end, remove_key_begin, DEFAULT_KEY_SIZE * (hdr.count - i - 1));

    void* remove_rid_end = base + sizeof(rid) * i;
    void* remove_rid_begin = base + sizeof(rid) * (i + 1);
    int len = DEFAULT_KEY_SIZE * (hdr.count - 1) + sizeof(rid) * (hdr.count - i - 1); 
    memmove(remove_rid_end, remove_key_begin, len);

    hdr.count--;
    hdr.free_start -= (sizeof(rid) + DEFAULT_KEY_SIZE);
    hdr.free_space += (sizeof(rid) + DEFAULT_KEY_SIZE);
    return true;
}

bool index_format::remove(key_t key, const rid& id)
{
    int i = find_in_leaf(key);
    if (i == -1)
        return false;
    
    void* remove_key_end = base + sizeof(rid) * hdr.count + DEFAULT_KEY_SIZE * i;
    void* remove_key_begin = base + sizeof(rid) * hdr.count + DEFAULT_KEY_SIZE * (i + 1);
    memmove(remove_key_end, remove_key_begin, DEFAULT_KEY_SIZE * (hdr.count - i - 1));

    void* remove_rid_end = base + sizeof(rid) * i;
    void* remove_rid_begin = base + sizeof(rid) * (i + 1);
    int len = DEFAULT_KEY_SIZE * (hdr.count - 1) + sizeof(rid) * (hdr.count - i - 1); 
    memmove(remove_rid_end, remove_key_begin, len);

    hdr.count--;
    hdr.free_start -= (sizeof(rid) + DEFAULT_KEY_SIZE);
    hdr.free_space += (sizeof(rid) + DEFAULT_KEY_SIZE);
    return true;
}

/*
while (C is not a leaf node) begin
    Let i = smallest number such that V <= C.Ki
    if there is no such number i then begin
        Let Pm = last non-null pointer in the node
        Set C = C.Pm
    end
    else if (V = C.Ki)
        then Set C = C.P(i+1)
    else
        C = C.Pi // V < C.Ki
end
*/
// rid0 key0 rid1 key1 rid2 (key2)
int index_format::find_in_internal(key_t key) const
{
    int ret = -1;
    for(size_t i = 0; i < hdr.count-1; i++)
    {
        if (key > keys[i])
            continue;
        else if (keys <= keys[i])
        {
            ret = i;
            break;
        }
    }
    if (ret == -1)
        return hdr.count-1;
    else if(key == keys[ret])
        return ret + 1;
    else
        return ret;
}


int index_format::find_in_leaf(key_t key) const
{
    // set L to 0 and R to n-1
    int left = 0;
    // should not count in the last node which is just a dummy key
    int right = hdr.count-2;
    int mid;

    // if L > R, the search terminates as unsuccessful.
    if (left > right)
        return -1;
    
    while(left <= right){
        // set m (the position of the middle element) to the
        // floor of (L + R) / 2, 
        mid = (left + right) / 2;
        int result = cmptr->compare({keys[mid], key_len}, {key, key_len});     
        
        // if Am < T, set L to m + 1
        if (result < 0)
            left = mid+1;
        // if Am > T, set R to m - 1
        else if(result > 0)
            right = mid-1;
        // Now Am = T, the search is down, return m
        else
            return mid;
    }

    return -1;
}

bool index_format::enough() const
{
    return hdr.count > 0;
}