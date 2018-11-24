#include "channel.h"

#include <unistd.h>

channel::channel(int fd, poller* poller): fd_{fd}, poller_{poller} {}

channel::~channel() {}

int channel::fd() const
{
    return fd_;
}

short channel::events() const
{
    return events_;
}

void channel::enable_read(bool enable)
{
    if (enable)
        events_ |= READ_EVENT;
    else
        events_ &= ~READ_EVENT;

    poller_->update_channel(this);
}

bool channel::read_enabled() const
{
    return events_ & READ_EVENT;
}

void channel::on_read(const callback& cb)
{
    enable_read(true);
    this->read_cb_ = cb;
}

void channel::handle_read() const
{
    read_cb_();
}

void channel::enable_write(bool enable)
{  
    if (enable)
        events_ |= WRITE_EVENT;
    else
        events_ &= ~WRITE_EVENT;

    poller_->update_channel(this);
}

bool channel::write_enabled() const
{
    return events_ & WRITE_EVENT;
}

void channel::on_write(const callback& cb)
{
    enable_write(true);
    this->write_cb_ = cb;
}

void channel::handle_write() const
{
    write_cb_();
}

void channel::close()
{
    if (fd_ >= 0) {
        poller_->remove_channel(this);
        ::close(fd_);
        fd_ = -1;
    }   
}