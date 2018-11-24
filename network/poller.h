#pragma once

#include <set>

#ifdef __linux__
#include <sys/epoll.h>
#endif

#ifdef __APPLE__
#include <sys/types.h>
#include <sys/event.h>
#include <sys/time.h>
#endif

/*
        wait for events
            ________
           /        \
          +  reactor +
          |   loop   |
          +          +
           \        /
            ^      |
            |      v        |
------------+------+--------|----------------
            |      |        v
            +      +
             \    / 
              +--+

*/

constexpr const int MAX_EVENTS{1 << 10};

class channel;

class poller
{
protected:
    int poller_fd_;
    int last_active_;
    std::set<channel *> channels_;
public:
    poller();
    virtual ~poller();
    poller(const poller& poller) = delete;
    poller& operator=(const poller& poller) = delete;

    virtual void add_channel(channel* chan) = 0;
    virtual void remove_channel(channel* chan) = 0;
    virtual void update_channel(channel* chan) = 0;
    /*
    x  max timeout, if greater than this value, whether or not it has event, it will return
    0  return immediately, whether event accured or not
    -1 block indefinitely
    */
    virtual void loop_once(int timeout) = 0;
};

poller* create_poller();

#ifdef __linux__
class epoll_poller : public poller
{
private:
    struct epoll_event events_[MAX_EVENTS];
public:
    poller();
    ~poller();
    
    void add_channel(channel* chan) override;
    void update_channel(channel* chan) override;
    void remove_channel(channel* chan) override;
    void loop_once(int timeout) override;
};
#endif

#ifdef __APPLE__
class kqueue_poller : public poller
{
private:
    struct kevent events_[MAX_EVENTS];
public:
    kqueue_poller();
    ~kqueue_poller();

    void add_channel(channel* chan) override;
    void update_channel(channel* chan) override;
    void remove_channel(channel* chan) override;
    void loop_once(int timeout) override;
};
#endif