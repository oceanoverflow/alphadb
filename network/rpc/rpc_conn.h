#pragma once

#include "conn.h"
#include "codec.h"

#include <map>
#include <mutex>
#include <atomic>
#include <functional>

class rpc_conn: public conn
{
private:
    codec codec_;
    std::mutex mutex_;
    std::atomic<rpc_id_t> rpc_id_;
    std::map<rpc_id_t, std::function<void()>> futures_;
public:
    rpc_conn(int fd, poller* poller);
    rpc_conn(const std::pair<std::string, short>& endpoint, poller* poller);
    ~rpc_conn();

    codec codec() const;

    using conn::handle_read;
    using conn::handle_write;

    template<typename T, typename A, typename B>
    rpc_id_t call(const char* str, const A& args, int id, std::function<void(int, B)> callback);

    void cancel(rpc_id_t rpcid);
};