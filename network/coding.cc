#include "coding.h"

int varint::encode_int(char *const buf, int64_t val)
{
    uint64_t uvalue = static_cast<uint64_t>(val < 0 ? ~(val << 1) : (val << 1));
    return encode_uint(buf, uvalue); 
}

int64_t varint::decode_varint(const char* const data, int& bytes)
{
    uint64_t uvalue = decode_uvarint(data, bytes); 
    return static_cast<int64_t>(uvalue & 1 ? ~(uvalue >> 1) : (uvalue >> 1));
}

int varint::encode_uint(char *const buf, uint64_t val)
{
    int i = 0;

    do {
        uint8_t next = val & 0x7F;
        val >>= 7;
        if (val)
            next |= 0x80;
        buf[i++] = next;
    } while (val);

    return i;
}

uint64_t varint::decode_uvarint(const char* const data, int& bytes)
{
    int i = 0;
    int shift = 0;
    uint64_t val = 0;
    
    do {
        val |= static_cast<uint64_t>(data[i] & 0x7F) << shift;
        shift += 7;
    } while ( (data[i++] & 0x80) != 0 );

    bytes = i;
    return val;
}


