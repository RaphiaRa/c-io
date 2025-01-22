/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_ACCEPTOR_H
#define IO_ACCEPTOR_H

#include <io/accept.h>
#include <io/config.h>
#include <io/context.h>
#include <io/descriptor.h>
#include <io/err.h>
#include <io/socket.h>
#include <io/task.h>

typedef struct io_Acceptor {
    io_Descriptor base;
} io_Acceptor;

DEFINE_DESCRIPTOR_WRAPPERS(io_Acceptor, io_Descriptor)

IO_INLINE(void)
io_Acceptor_init(io_Acceptor* acceptor, io_Context* ctx)
{
    io_Descriptor_init(&acceptor->base, ctx);
}

IO_INLINE(io_Err)
io_Acceptor_async_accept(io_Acceptor* acceptor, io_Socket* socket, io_AcceptCallback callback, void* user_data)
{
    io_AcceptOp* op = io_AcceptOp_create(&acceptor->base, &socket->base, callback, user_data);
    if (!op) {
        return io_SystemErr(IO_ENOMEM);
    }
    io_Handle_submit(acceptor->base.handle, &op->base);
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_Acceptor_accept(io_Acceptor* acceptor, io_Socket* socket)
{
    return io_perform_accept(&acceptor->base, &socket->base);
}

IO_INLINE(void)
io_Acceptor_deinit(io_Acceptor* acceptor)
{
    io_Descriptor_close(&acceptor->base);
}

#define DEFINE_ACCEPT_WRAPPERS(A, S)                                                          \
    IO_INLINE(io_Err)                                                                         \
    A##_async_accept(A* acceptor, S* socket, io_AcceptCallback callback, void* user_data)     \
    {                                                                                         \
        return io_Acceptor_async_accept(&acceptor->base, &socket->base, callback, user_data); \
    }                                                                                         \
                                                                                              \
    IO_INLINE(io_Err)                                                                         \
    A##_accept(A* acceptor, S* socket)                                                        \
    {                                                                                         \
        return io_Acceptor_accept(&acceptor->base, &socket->base);                            \
    }

#endif
