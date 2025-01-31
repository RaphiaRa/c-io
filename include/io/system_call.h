#ifndef IO_SYSTEM_CALL_H
#define IO_SYSTEM_CALL_H

#include <io/config.h>
#include <io/err.h>

#if IO_OS_POSIX
#if !IO_MOCKING

#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

IO_INLINE(ssize_t)
io_read(int fd, void* buf, size_t count)
{
    return read(fd, buf, count);
}

IO_INLINE(ssize_t)
io_write(int fd, const void* buf, size_t count)
{
    return write(fd, buf, count);
}

IO_INLINE(int)
io_close(int fd)
{
    return close(fd);
}

IO_INLINE(int)
io_socket(int domain, int type, int protocol)
{
    return socket(domain, type, protocol);
}

IO_INLINE(int)
io_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return bind(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_listen(int sockfd, int backlog)
{
    return listen(sockfd, backlog);
}

IO_INLINE(int)
io_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    return accept(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_connect(int sockfd, const void* addr, socklen_t addrlen)
{
    return connect(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_pipe(int pipefd[2])
{
    return pipe(pipefd);
}

#define io_fcntl(...) fcntl(__VA_ARGS__)

#if IO_WITH_POLL

#include <poll.h>

IO_INLINE(int)
io_poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
    return poll(fds, nfds, timeout);
}

#endif // IO_WITH_POLL
#else  // IO_MOCKING

#include <fcntl.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

typedef struct io_MockSystemCall {
    ssize_t (*read)(int fd, void* buf, size_t count);
    ssize_t (*write)(int fd, const void* buf, size_t count);
    int (*close)(int fd);
    int (*socket)(int domain, int type, int protocol);
    int (*bind)(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
    int (*listen)(int sockfd, int backlog);
    int (*accept)(int sockfd, struct sockaddr* addr, socklen_t* addrlen);
    int (*connect)(int sockfd, const struct sockaddr* addr, socklen_t addrlen);
    int (*pipe)(int pipefd[2]);
    int (*fcntl)(int fd, int cmd, ...);
#if IO_WITH_POLL
    int (*poll)(struct pollfd* fds, nfds_t nfds, int timeout);
#endif
} io_MockSystemCall;

extern io_MockSystemCall io_mock_system_call;

IO_INLINE(ssize_t)
io_read(int fd, void* buf, size_t count)
{
    return io_mock_system_call.read(fd, buf, count);
}

IO_INLINE(ssize_t)
io_write(int fd, const void* buf, size_t count)
{
    return io_mock_system_call.write(fd, buf, count);
}

IO_INLINE(int)
io_close(int fd)
{
    return io_mock_system_call.close(fd);
}

IO_INLINE(int)
io_socket(int domain, int type, int protocol)
{
    return io_mock_system_call.socket(domain, type, protocol);
}

IO_INLINE(int)
io_bind(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return io_mock_system_call.bind(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_listen(int sockfd, int backlog)
{
    return io_mock_system_call.listen(sockfd, backlog);
}

IO_INLINE(int)
io_accept(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    return io_mock_system_call.accept(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_connect(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    return io_mock_system_call.connect(sockfd, addr, addrlen);
}

IO_INLINE(int)
io_pipe(int pipefd[2])
{
    return io_mock_system_call.pipe(pipefd);
}

#define io_fcntl(...) io_mock_system_call.fcntl(__VA_ARGS__)

#if IO_WITH_POLL
IO_INLINE(int)
io_poll(struct pollfd* fds, nfds_t nfds, int timeout)
{
    return io_mock_system_call.poll(fds, nfds, timeout);
}
#endif // IO_WITH_POLL

#endif // IO_MOCKING

#endif // IO_OS_POSIX

#endif
