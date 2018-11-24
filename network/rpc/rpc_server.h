#pragma once

#include <map>
#include <atomic>
#include <functional>

#include "rpc.h"
#include "server.h"

class rpc_server: public server
{
private:  
    std::map<int, rpc *> services_;
    void handle_accept();
protected:
    std::atomic<int> rpc_count_;
public:
    rpc_server(dispatcher* d, short port);
    virtual ~rpc_server();

    virtual void start() override;
    virtual void shutdown() override;

    template<typename T, typename R1, typename R2>
    void Register(const char* str, T* obj, std::function<void(T*, const R1&, R2&)> func);
    int count() const;
};
