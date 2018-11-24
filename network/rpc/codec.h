#pragma once

#include "buffer.h"
#include "coding.h"

typedef uint64_t rpc_id_t;

class codec
{
private:
    buffer* read_buf_;
    buffer* write_buf_;
public:
    codec(buffer* read_buf, buffer* write_buf);
    ~codec();
    codec(const codec& codec);
    codec& operator=(const codec& codec);
    
    template<typename T> void marshal(const rpc_id_t&, const T& args);
    template<typename T> void unmarshal(int& id, T& args);

    buffer* read_buffer() const;
    buffer* write_buffer() const;

    bool is_ready() const;
};