#include "poller.h"
#include "channel.h"

#include <unistd.h>

#ifdef __linux__

/*
struct epoll_event {
    __uint32_t   events; // epoll events
    epoll_data_t data;   // user data variable
};

typedef union epoll_data {
    void*      ptr;
    int        fd;
    __uint32_t u32;
    __uint64_t u64;
} epoll_data_t;

EPOLLIN
EPOLLOUT
EPOLLPRI
EPOLLERR
EPOLLHUP
EPOLLET
EPOLLONESHOT
*/

epoll_poller::epoll_poller()
{
    poller_fd_ = epoll_create1(EPOLL_CLOEXEC);
}

epoll_poller::~epoll_poller()
{ 
    while(channels_.size())
        (*channels_.begin())->close();
    close(poller_fd_);
}

void epoll_poller::add_channel(channel* chan)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.data.ptr = chan;
    ev.events = chan->events();
    epoll_ctl(poller_fd_, EPOLL_CTL_ADD, chan->fd(), &ev);
    channels_.insert(chan);
}

void epoll_poller::update_channel(channel* chan)
{
    struct epoll_event ev;
    memset(&ev, 0, sizeof(epoll_event));
    ev.events = chan->events();
    ev.data.ptr = chan;
    epoll_ctl(poller_fd_, EPOLL_CTL_MOD, chan->fd(), &event);
}

void epoll_poller::remove_channel(channel* chan)
{
    struct epoll_event ev;
    memset(&event, 0, sizeof(ev));
    ev.events = chan->events();
    ev.data.ptr = chan;
    epoll_ctl(poller_fd_, EPOLL_CTL_DEL, chan->fd(), &ev);
    channel_.erase(chan);
}

void epoll_poller::loop_once(int timeout)
{
    last_active_ = epoll_wait(poller_fd_, events_, MAX_EVENTS, timeout);
    
    while(--last_active_ >= 0){
        int i = last_active_;
        channel* chan = static_cast<channel *>(events_[i].data.ptr);
        int events = events_[i].events;
        
        if (chan != nullptr) {
            if (events & (POLLIN | POLLERR))
                chan->handle_read();
            else if (events & POLLOUT)
                chan->handle_write();
        }
    }  
}

poller* create_poller()
{
    return new epoll_poller();
}
#endif

#ifdef __APPLE__
kqueue_poller::kqueue_poller()
{
    poller_fd_ = kqueue();
}

kqueue_poller::~kqueue_poller()
{
    while(channels_.size())
        (*channels_.begin())->close();
    close(poller_fd_);
}

void kqueue_poller::add_channel(channel* chan)
{
    int n = 0;
    struct kevent ev[2]; 
    struct timespec nullts{0, 0};

    if (chan->read_enabled())
        EV_SET(&ev[n++], chan->fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, chan);
    
    if (chan->write_enabled())
        EV_SET(&ev[n++], chan->fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, chan);

    kevent(poller_fd_, ev, n, nullptr, 0, &nullts);
    channels_.insert(chan);
}

void kqueue_poller::update_channel(channel* chan)
{ 
    int n = 0;
    struct kevent ev[2];
    struct timespec nullts{0, 0};
    
    if (chan->read_enabled())
        EV_SET(&ev[n++], chan->fd(), EVFILT_READ, EV_ADD | EV_ENABLE, 0, 0, chan);
    else
        EV_SET(&ev[n++], chan->fd(), EVFILT_READ, EV_DELETE, 0, 0, chan);

    if (chan->write_enabled())
        EV_SET(&ev[n++], chan->fd(), EVFILT_WRITE, EV_ADD | EV_ENABLE, 0, 0, chan);
    else
        EV_SET(&ev[n++], chan->fd(), EVFILT_WRITE, EV_DELETE, 0, 0, chan);

    kevent(poller_fd_, ev, n, nullptr, 0, &nullts);
}

void kqueue_poller::remove_channel(channel* chan)
{
    channels_.erase(chan);
    
    for(int i = last_active_; i >= 0; i--)
    {    
        if (chan == events_[i].udata) {
            events_[i].udata = nullptr;
            break;
        }    
    } 
}

void kqueue_poller::loop_once(int timeout)
{
    struct timespec to{timeout / 1000, (timeout % 1000) * 1000 * 1000};

    last_active_ = kevent(poller_fd_, nullptr, 0, events_, MAX_EVENTS, &to);
    
    while(--last_active_ >= 0){
        int i = last_active_;
        channel* chan = static_cast<channel *>(events_[i].udata);
        struct kevent &kev = events_[i];
        
        if (chan != nullptr) {       
            if ((kev.flags & EV_EOF) || chan->read_enabled())
                chan->handle_read();
            else if(!(kev.flags & EV_EOF) && chan->write_enabled())
                chan->handle_write();
        }  
    }  
}

poller* create_poller()
{
    return new kqueue_poller();
}

#endif