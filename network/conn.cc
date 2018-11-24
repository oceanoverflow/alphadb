#include "conn.h"
#include "net.h"

#include <cerrno>
#include <cstring>

#include <sys/types.h>
#include <sys/uio.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>

conn::conn(int fd, poller* poller)
{
    set_non_blocking(fd, true);
    channel_ = new channel(fd, poller);
    channel_->on_read([this]{ this->handle_read(); });
    channel_->on_write([this]{ this->handle_read(); });
}

conn::conn(const std::pair<std::string, short>& endpoint, poller* poller)
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(endpoint.second);
    addr.sin_addr.s_addr = inet_addr(endpoint.first.c_str());
    
    sock_ = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_ < 0)
        throw std::exception{};
    
    set_non_blocking(sock_, true);

    if (connect(sock_, (const sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::exception{};
    
    channel_ = new channel(sock_, poller);
    channel_->on_read([this]{ this->handle_read(); });
    channel_->on_write([this]{ this->handle_write(); });
}

conn::~conn() {
    close();
}

void conn::on_read(const callback& cb)
{
    read_cb_ = cb;
}

void conn::handle_read()
{
    ssize_t rb = 0;

    while(1){
        ssize_t r = read(sock_, read_buf_.put_begin(), read_buf_.size());
        if (r == 0)
           break;
        else if (r < 0) {
            if (errno == EINTR) 
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;     
        }  
        rb += r;
        read_buf_.advance_put(r);
    }

    if (read_cb_)
        read_cb_();
}

void conn::on_write(const callback& cb)
{
    write_cb_ = cb;
}

void conn::handle_write()
{
    ssize_t wb = 0;
    
    while(1){
        ssize_t w = write(sock_, write_buf_.get_begin(), write_buf_.size());
        
        if (w > 0) {
            wb += w;
            write_buf_.advance_get(w);
            continue;
        }
        else if (w == -1) { 
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
        }
        break;
    }
    
    if (write_cb_)
        write_cb_();
}

void conn::close()
{
    ::close(sock_);
    delete channel_;
}

void conn::send(buffer& buf)
{
    ssize_t wb = 0;
    
    while(1){
        ssize_t w = write(sock_, buf.get_begin(), buf.size());
        
        if (w > 0) {
            wb += w;
            buf.advance_get(w);
            continue;
        }
        else if (w == -1) { 
            if (errno == EINTR)
                continue;
            if (errno == EAGAIN || errno == EWOULDBLOCK)
                break;
        }
        break;
    }
}

void conn::send(const char* str, size_t len)
{
    write_buf_.put(str, len);
    send_output();
}

void conn::send(const std::string& str)
{
    conn::send(str.c_str(), str.length());
}

void conn::send(const char* str)
{
    conn::send(str, strlen(str));
}

void conn::send_output()
{
    conn::send(write_buf_);
}

buffer* conn::read_buffer()
{
    return &read_buf_;
}

buffer* conn::write_buffer()
{
    return &write_buf_;
}
