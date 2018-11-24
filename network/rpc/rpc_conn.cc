#include "rpc_conn.h"

#include <memory>
#include <future>
#include <iostream>

rpc_conn::rpc_conn(int fd, poller* poller) : rpc_id_(0), conn(fd, poller), codec_(&read_buf_, &write_buf_) {}

rpc_conn::rpc_conn(const std::pair<std::string, short>& endpoint, poller* poller)
: conn(endpoint, poller), codec_(&read_buf_, &write_buf_), rpc_id_(0)
{
    on_read([this]{
        while(!codec_.is_ready());
        
        int bytes;
        buffer* buf = codec_.read_buffer();
        rpc_id_t rpcid = varint::decode_uvarint(buf->get_begin(), bytes);
        buf->advance_get(bytes);

        auto it = futures_.find(rpcid);

        if (it != futures_.end()) {
            std::function<void()> func;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                func = futures_[rpcid];
                futures_.erase(it);
            }

            func();
        }
    });
}

rpc_conn::~rpc_conn() {}

codec rpc_conn::codec() const
{
    return codec_;
}

template<typename T, typename A, typename R>
rpc_id_t rpc_conn::call(const char* str, const A& args, int id, std::function<void(int, R)> callback)
{
    rpc_id_t rpcid = rpc_id_++;
    codec_.marshal(rpcid, args);
    send_output();
    
    auto task = std::make_shared<std::packaged_task<void()>>([this, &](){
        int id; 
        R r;
        codec_.unmarshal(id, r);
        callback(id, r); 
    });

    std::future<void> future = task->get_future();
    {
        std::unique_lock<std::mutex> lock(mutex_);
        futures_.insert({rpcid, [task]() { (*task)(); }});
    }

    return rpcid;
}

void rpc_conn::cancel(rpc_id_t id)
{
    std::unique_lock<std::mutex> lock(mutex_);
    auto it = futures_.find(id);
    
    if (it != futures_.end()) {
        futures_.erase(it);
    } 
}