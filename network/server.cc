#include "net.h"
#include "server.h"
#include "exceptions.h"

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

server::server(dispatcher* d, short port): dispatcher_{d}, port_{port} {}

server::~server() {
    shutdown();
}

/*
struct sockaddr_in {
    sa_family_t    sin_family; // address family: AF_INET
    in_port_t      sin_port;   // port in network byte order
    struct in_addr sin_addr;   // internet address
};

struct in_addr {
    uint32_t char s6_addr[16]; // address in network byte order
};

struct sockaddr_in6 { 
    sa_family_t     sin6_family;    // AF_INET6
    in_port_t       sin6_port;      // port number
    uint32_t        sin6_flowinfo;  // IPv6 flow information
    struct in6_addr sin6_addr;      // IPv6 address
    uint32_t        sin6_scope_id;  // scope ID 
};

struct in6_addr { 
    unsigned char   s6_addr[16];    // IPv6 address
};
*/

void server::start()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = PF_INET;
    addr.sin_port = htons(port_);
    addr.sin_addr.s_addr = inet_addr(SERVER_HOST);

    listen_fd_ = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listen_fd_ < 0)
        throw std::exception{};
    
    set_reuse_address(listen_fd_, true); 
    set_reuse_port(listen_fd_, true);
    
    if (bind(listen_fd_, (const struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::exception{};

    // socket() 函数创建的socket默认是主动类型的，listen函数将socket变为被动类型，等待客户的连接请求
    int ret = listen(listen_fd_, 5);
    if (ret < 0)
        throw std::exception{};
    
    listener_ = new channel(listen_fd_, dispatcher_->poller());
    listener_->on_read([this]{ this->handle_accept(); });
}

void server::shutdown()
{
    ::close(listen_fd_);
    delete listener_;
   
    for(auto& conn : conns_)
        conn->close();
}

void server::handle_accept()
{
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    socklen_t len = sizeof(struct sockaddr_in);
    int fd = accept(listen_fd_, (struct sockaddr *)&addr, &len);
    set_non_blocking(fd, true);
    conn* c = new conn(fd, dispatcher_->poller());  
    if (callback_)
        callback_(c);
    conns_.push_back(c);
}

void server::set_callback(std::function<void(conn *)> cb)
{
    callback_ = cb;
}