#pragma once

#include <string>
#include <utility>
#include <functional>

#include "buffer.h"
#include "channel.h"

using callback = std::function<void()>;

class poller;

class conn
{
protected:
    int sock_;
    buffer read_buf_;
    buffer write_buf_;
    channel* channel_;
    callback read_cb_;
    callback write_cb_; 
public:
    // server accept client connection
    conn(int fd, poller* poller);
    // client connect to server
    conn(const std::pair<std::string, short>& endpoint, poller* poller);
    conn(const conn& c) = delete;
    conn& operator=(const conn& c) = delete;
    virtual ~conn();

    void handle_read();
    void on_read(const callback& cb);

    void handle_write();
    void on_write(const callback& cb);

    void send(buffer& buf);
    void send_output();

    void send(const char* s);
    void send(const std::string& str);
    void send(const char* str, size_t len);

    buffer* read_buffer();
    buffer* write_buffer();

    void close();
};
