#include "codec.h"
#include "coding.h"

#include <vector>

codec::codec(buffer* read_buf, buffer* write_buf): read_buf_{read_buf}, write_buf_{write_buf} {}

codec::~codec() {}

template<typename T>
void codec::marshal(const rpc_id_t& rid, const T& args)
{
    *this << rid;
    *this << args;
}

template<typename T>
void codec::unmarshal(int& id, T& args)
{
    *this >> id;
    *this >> args;
}

buffer* codec::read_buffer() const
{
    return read_buf_;
}

buffer* codec::write_buffer() const
{
    return write_buf_;
}

bool codec::is_ready() const
{
    buffer* b = read_buffer();
    int bytes;
    int len = varint::decode_varint(b->get_begin(), bytes);
    
    if (len + bytes < b->size())
        return false;
    else
        return true;
}

codec& operator<<(codec& c, const rpc_id_t& rid)
{
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[10];
    bytes = varint::encode_uint(buf, static_cast<uint64_t>(rid));
    b->put(buf, bytes);
    return c;
}

codec& operator>>(codec& c, rpc_id_t& rid)
{
    buffer* b = c.read_buffer();
    int bytes;
    rid = static_cast<rpc_id_t>(varint::decode_uvarint(c.read_buffer()->get_begin(), bytes));
    return c;
}