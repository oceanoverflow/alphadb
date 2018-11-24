#pragma once

#include <cstdint> 

// Varint encoding/decoding

/*
int i = 1 << 5;
0010,0000                                         (varint) p.s. 3 byte less
0000,0000 0000,0000 0000,0000 0010,0000           (fixed length encoding)
---------------------------------------------------------------------------

int i = 1 << 10;
1000,0000 1000,1000                               (varint) p.s. 2 byte less                               
0000,0000 0000,0000 0000,0100 0000,0000           (fixed length encoding)
---------------------------------------------------------------------------

int i = 1 << 16;
1000,0000 1000,0000 0000,0100                     (varint) p.s. 1 byte less
0000,0000 0000,0001 0000,0000 0000,0000           (fixed length encoding)
---------------------------------------------------------------------------

int i = 1 << 21;
1000,0000 1000,0000 1000,0000 0000,0001           (varint) p.s. equal ===
0000,0000 0010,0000 0000,0000 0000,0000           (fixed length encoding)
---------------------------------------------------------------------------

int i = 1 << 28;
1000,0000 1000,0000 1000,0000 1000,0000 0000,0001 (varint) p.s. 1 byte more
0001,0000 0000,0000 0000,0000 0000,0000           (fixed length encoding)
---------------------------------------------------------------------------
*/

struct varint
{
    static int encode_int(char* const buf, int64_t val);
    static int64_t decode_varint(const char* const data, int& bytes);

    static int encode_uint(char* const buf, uint64_t val);
    static uint64_t decode_uvarint(const char* const data, int& bytes);
};