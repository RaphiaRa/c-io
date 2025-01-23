#ifndef IO_SYSCALL_STUBS_H
#define IO_SYSCALL_STUBS_H

#include <errno.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>

#include <io/atomic.h>

extern int stub_socket_num;

static inline int
fcntl_stub_success(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    return 0;
}

static inline int
fcntl_stub_einval(int fd, int cmd, ...)
{
    (void)fd;
    (void)cmd;
    errno = EINVAL;
    return -1;
}

static inline ssize_t
read_stub_success(int fd, void* buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return io_atomic_fetch_add(&stub_socket_num, 1);
}

static inline ssize_t
read_stub_ebadf(int fd, void* buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    errno = EBADF;
    return -1;
}

static inline ssize_t
write_stub_success(int fd, const void* buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    return count;
}

static inline ssize_t
write_stub_ebadf(int fd, const void* buf, size_t count)
{
    (void)fd;
    (void)buf;
    (void)count;
    errno = EBADF;
    return -1;
}

static inline int
connect_stub_success(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    return io_atomic_fetch_add(&stub_socket_num, 1);
}

static inline int
connect_stub_ebadf(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    errno = EBADF;
    return -1;
}

static inline int
accept_stub_success(int sockfd, struct sockaddr* addr, socklen_t* addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    return io_atomic_fetch_add(&stub_socket_num, 1);
}

static inline int
poll_stub_success(struct pollfd* fds, nfds_t nfds, int timeout)
{
    (void)timeout;
    // we mark everything as ready
    for (nfds_t i = 0; i < nfds; ++i) {
        fds[i].revents = fds[i].events;
    }
    return nfds;
}

static inline int
socket_stub_success(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    return io_atomic_fetch_add(&stub_socket_num, 1);
}

static inline int
socket_stub_emfile(int domain, int type, int protocol)
{
    (void)domain;
    (void)type;
    (void)protocol;
    errno = EMFILE;
    return -1;
}

static inline int
bind_stub_success(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    return 0;
}

static inline int
bind_stub_eaddrinuse(int sockfd, const struct sockaddr* addr, socklen_t addrlen)
{
    (void)sockfd;
    (void)addr;
    (void)addrlen;
    errno = EADDRINUSE;
    return -1;
}

static inline int
listen_stub_success(int sockfd, int backlog)
{
    (void)sockfd;
    (void)backlog;
    return 0;
}

static inline int
listen_stub_ebadf(int sockfd, int backlog)
{
    (void)sockfd;
    (void)backlog;
    errno = EBADF;
    return -1;
}

static inline int
pipe_stub_success(int pipefd[2])
{
    pipefd[0] = 0;
    pipefd[1] = 1;
    return 0;
}

static inline int
close_stub_success(int fd)
{
    (void)fd;
    return 0;
}

#endif
