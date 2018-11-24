#include "rpc_args.h"

codec& operator>>(codec& c, request_vote_args& args)
{
    buffer* b = c.read_buffer();
    char len[2];
    b->get(len, 2);
    int reallen = 0; // here some magic transformation
    int bytes = 0;
    args.term = varint::decode_uvarint(b->get_begin(), bytes);
    args.candidate_id = varint::decode_uvarint(b->get_begin(), bytes);
    args.last_log_index = varint::decode_uvarint(b->get_begin(), bytes);
    args.last_log_term = varint::decode_uvarint(b->get_begin(), bytes);
    
    if (bytes != reallen)
        b->unget(bytes+2);
    return c;
}

codec& operator>>(codec& c, request_vote_reply& args)
{
    buffer* b = c.read_buffer();
    char len[2];
    b->get(len, 2);
    int reallen = 0; // here some magic transformation
    int bytes = 0;
    args.term = varint::decode_uvarint(b->get_begin(), bytes);
    args.vote_granted = varint::decode_uvarint(b->get_begin(), bytes);
    
    if (bytes != reallen)
        b->unget(bytes+2); 
    return c;
}

codec& operator>>(codec& c, append_entries_args& args)
{
    buffer* b = c.read_buffer();
    char len[2];
    b->get(len, 2);
    int reallen = 0; // here some magic transformation
    int bytes = 0;
    args.term = varint::decode_uvarint(b->get_begin(), bytes);
    args.leader_id = varint::decode_uvarint(b->get_begin(), bytes);
    args.prev_log_index = varint::decode_uvarint(b->get_begin(), bytes);
    args.prev_log_term = varint::decode_uvarint(b->get_begin(), bytes);
    args.leader_commit = varint::decode_uvarint(b->get_begin(), bytes);

    char len2[2];
    b->get(len2, 2);

    // decode logs lots of them, fxxk
    
    if (bytes != reallen)
        b->unget(bytes+2);
    return c;
}

codec& operator>>(codec& c, append_entries_reply& args)
{
    buffer* b = c.read_buffer();
    char len[2];
    b->get(len, 2);
    int reallen = 0; // here some magic transformation
    int bytes = 0;
    args.term = varint::decode_uvarint(b->get_begin(), bytes);
    args.success = varint::decode_uvarint(b->get_begin(), bytes);
    
    if (bytes != reallen)
        b->unget(bytes+2); 
    return c;
}

codec& operator<<(codec& c, const request_vote_args& args)
{   
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[40];
    bytes += varint::encode_uint(buf, args.term);
    bytes += varint::encode_uint(buf, args.candidate_id);
    bytes += varint::encode_uint(buf, args.last_log_index);
    bytes += varint::encode_uint(buf, args.last_log_index);
    b->put(uint16_t(bytes));
    b->put(buf, bytes);
    return c;
}

codec& operator<<(codec& c, const request_vote_reply& args)
{
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[20];
    bytes += varint::encode_uint(buf, args.term);
    bytes += varint::encode_uint(buf, args.vote_granted);
    b->put(uint16_t(bytes));
    b->put(buf, bytes);
    return c; 
}

codec& operator<<(codec& c, const append_entries_args& args)
{
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[50];
    bytes += varint::encode_uint(buf, args.term);
    bytes += varint::encode_uint(buf, args.leader_id);
    bytes += varint::encode_uint(buf, args.prev_log_index);
    bytes += varint::encode_uint(buf, args.prev_log_term);
    bytes += varint::encode_uint(buf, args.leader_commit);

    uint16_t size = args.log_entries.size();

    char bbuf[1024];
    int bytes2 = 0;
    
    for(size_t i = 0; i < size; i++)
    {
        bytes2 += varint::encode_uint(bbuf, args.log_entries[i].index);
        bytes2 += varint::encode_uint(bbuf, args.log_entries[i].term);
    }
    
    b->put(uint16_t(bytes+bytes2));
    b->put(uint16_t(size));
    b->put(buf, bytes);
    b->put(bbuf, bytes2);
    return c;
}

codec& operator<<(codec& c, const append_entries_reply& args)
{
    buffer* b = c.write_buffer();
    int bytes = 0;
    char buf[20];
    bytes += varint::encode_uint(buf, args.term);
    bytes += varint::encode_uint(buf, args.success);
    b->put(uint16_t(bytes));
    b->put(buf, bytes);
    return c; 
}