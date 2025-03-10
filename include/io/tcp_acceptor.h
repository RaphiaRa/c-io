/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_TCP_ACCEPTOR_H
#define IO_TCP_ACCEPTOR_H

#include <io/config.h>

#include <io/config.h>

#include <io/acceptor.h>
#include <io/context.h>
#include <io/err.h>
#include <io/system_call.h>
#include <io/system_err.h>
#include <io/tcp_socket.h>

#include <arpa/inet.h>
#include <sys/socket.h>

typedef struct io_TcpAcceptor {
    io_Acceptor base;
} io_TcpAcceptor;

DEFINE_DESCRIPTOR_WRAPPERS(io_TcpAcceptor, io_Acceptor)
DEFINE_ACCEPT_WRAPPERS(io_TcpAcceptor, io_TcpSocket)

IO_INLINE(io_Err)
io_parse_addr(const char* addr, struct sockaddr_in* sockaddr,
              struct sockaddr_in6* sockaddr6,
              struct sockaddr** sockaddr_out,
              socklen_t* sockaddr_len)
{
    const char* portstr = strrchr(addr, ':');
    if (!portstr) {
        return io_SystemErr(IO_EINVAL);
    }
    in_port_t port = (in_port_t)atoi(portstr + 1);
    if (port == 0) {
        return io_SystemErr(IO_EINVAL);
    }

    char ipstr[64] = {0};
    snprintf(ipstr, sizeof(ipstr), "%.*s", (int)(portstr - addr), addr);
    if (ipstr[0] == '[' && ipstr[strlen(ipstr) - 1] == ']') {
        ipstr[strlen(ipstr) - 1] = '\0';
        if (inet_pton(AF_INET6, &ipstr[1], &sockaddr6->sin6_addr) == 1) {
            sockaddr6->sin6_family = AF_INET6;
            sockaddr6->sin6_port = port;
            *sockaddr_out = (struct sockaddr*)sockaddr6;
            *sockaddr_len = sizeof(*sockaddr6);
            return IO_ERR_OK;
        }
    } else {
        if (inet_pton(AF_INET, ipstr, &sockaddr->sin_addr) == 1) {
            sockaddr->sin_family = AF_INET;
            sockaddr->sin_port = port;
            *sockaddr_out = (struct sockaddr*)sockaddr;
            *sockaddr_len = sizeof(*sockaddr);
            return IO_ERR_OK;
        }
    }
    return io_SystemErr(IO_EINVAL);
}

IO_INLINE(io_Err)
io_TcpAcceptor_init(io_TcpAcceptor* acceptor, io_Context* ctx, const char* addr)
{
    io_Acceptor_init(&acceptor->base, ctx);
    int fd = io_socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        return io_SystemErr(errno);
    }
    struct sockaddr_in sockaddr = {0};
    struct sockaddr_in6 sockaddr6 = {0};
    struct sockaddr* sockaddr_out = NULL;
    socklen_t sockaddr_len = 0;
    io_Err err = IO_ERR_OK;
    if ((err = io_parse_addr(addr, &sockaddr, &sockaddr6, &sockaddr_out, &sockaddr_len))) {
        goto cleanup_socket;
    }
    if (io_bind(fd, (struct sockaddr*)sockaddr_out, sockaddr_len) == -1) {
        err = io_SystemErr(errno);
        goto cleanup_socket;
    }
    if (io_listen(fd, 128) == -1) {
        err = io_SystemErr(errno);
        goto cleanup_socket;
    }
    io_Acceptor_set_fd(&acceptor->base, fd);
    return err;
cleanup_socket:
    io_close(fd);
    return err;
}

IO_INLINE(void)
io_TcpAcceptor_deinit(io_TcpAcceptor* acceptor)
{
    io_Acceptor_deinit(&acceptor->base);
}

#endif
