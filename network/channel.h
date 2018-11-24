#pragma once

#include "poller.h"
#include <functional>

#ifdef __linux__
#include <sys/epoll.h>
const int READ_EVENT{EPOLLIN};
const int WRITE_EVENT{EPOLLOUT};
#elif defined(__APPLE__)
#include <sys/event.h>
const int READ_EVENT{EVFILT_READ};
const int WRITE_EVENT{EVFILT_WRITE};
#endif

using callback = std::function<void()>;

class channel
{
private:
    int fd_;
    short events_;
    poller* poller_;

    callback read_cb_;
    callback write_cb_;
public:
    channel(int fd, poller* poller);
    channel(const channel&) = delete;
    channel& operator=(const channel*) = delete;
    ~channel();

    int fd() const;
    short events() const;

    void enable_read(bool enable);
    bool read_enabled() const;
    void on_read(const callback& cb);
    void handle_read() const;
    
    void enable_write(bool enable);
    bool write_enabled() const;
    void on_write(const callback& cb);
    void handle_write() const;

    void close();
};
