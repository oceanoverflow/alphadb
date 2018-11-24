#include "net.h"
#include "codec.h"
#include "rpc_conn.h"
#include "rpc_server.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

rpc_server::rpc_server(dispatcher* d, short port) : server(d, port), rpc_count_{0} {}

rpc_server::~rpc_server() {
    for(auto& i : services_)
        delete i.second;
}

void rpc_server::start()
{
    server::start();
    listener_->on_read([this]{ this->handle_accept(); });
}

void rpc_server::shutdown()
{
    server::shutdown();
}

int rpc_server::count() const
{
    return rpc_count_.load();
}

template<typename T, typename R1, typename R2>
void rpc_server::Register(const char* str, T* obj, std::function<void(T*, const R1&, R2&)> func)
{
    rpc* r = new rpc();
    r->set(str, obj, func);
    int hashcode = r->hash();
    if (services_.find(hashcode) == services_.end())
        services_.insert(hashcode, r);
}

void rpc_server::handle_accept()
{
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    socklen_t len = sizeof(addr);
    int fd = accept(fd_, (struct sockaddr*)&addr, &len);
    set_non_blocking(fd, true);

    rpc_conn* conn = new rpc_conn(fd, dispatcher_->poller());
    conns_.push_back(conn);
    conn->on_read([this, &conn]() {
        codec cc = conn->codec();
        rpc_id_t rpcid;
        cc >> rpcid;
        auto it = services_.find(rpcid);
        
        if (it != services_.end()) {
            rpc* rpc = it->second;
            rpc->set_codec(cc);
            (*rpc)();
            rpc_count_++;
        }
        conn->send_output();
    });
}