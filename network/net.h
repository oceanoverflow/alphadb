#pragma once

#include <fcntl.h>
#include <sys/socket.h>

void set_non_blocking(int fd, bool value = true)
{ 
    int flags = fcntl(fd, F_GETFL, 0);
    value ? fcntl(fd, F_SETFL, flags | O_NONBLOCK) : fcntl(fd, F_SETFL, flags & ~O_NONBLOCK);
}

void set_reuse_address(int fd, bool value = true)
{
    int flag = value;
    setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(int));
}

void set_reuse_port(int fd, bool value= true)
{
    int flag = value;
    setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &flag, sizeof(int));
}

void add_flag(int fd, int flag)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | flag);
}
