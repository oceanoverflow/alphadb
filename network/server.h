#pragma once

#include <vector>
#include <string>

#include "conn.h"
#include "channel.h"
#include "dispatcher.h"

const char* SERVER_HOST{"localhost"};

class server
{
private:
    void handle_accept();
protected:
    short port_;
    int listen_fd_;
    channel* listener_;
    dispatcher* dispatcher_;
    std::vector<conn *> conns_;
    std::function<void(conn *)> callback_;
public:
    server(dispatcher* d, short port);
    server(const server& s) = delete;
    server& operator=(const server& s) = delete;
    virtual ~server();

    virtual void start();
    virtual void shutdown();

    void set_callback(std::function<void(conn *)> cb);
};
