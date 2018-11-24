#include "log.h"

#include "codec.h"
#include "coding.h"

codec& operator<<(codec& c, const struct log& args)
{
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[20];
    bytes += varint::encode_uint(buf, args.index);
    bytes += varint::encode_uint(buf, args.term);
    b->put(char(bytes));
    b->put(char(bytes>>8));
    b->put(buf, bytes);
    return c;
}

codec& operator>>(codec& c, struct log& args)
{
    buffer* b = c.read_buffer();
    char len = b->get();
    int bytes = 0;
    args.index = varint::decode_uvarint(b->get_begin(), bytes);
    args.term = varint::decode_uvarint(b->get_begin(), bytes);
    
    if (bytes != len)
        b->unget(bytes+2); 
    return c;
}