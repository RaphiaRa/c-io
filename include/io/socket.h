/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_SOCKET_H
#define IO_SOCKET_H

#include <io/config.h>
#include <io/descriptor.h>
#include <io/read.h>
#include <io/task.h>
#include <io/write.h>

#include <stddef.h>

typedef struct io_Socket {
    io_Descriptor base;
} io_Socket;

DEFINE_DESCRIPTOR_WRAPPERS(io_Socket, io_Descriptor)

IO_INLINE(void)
io_Socket_init(io_Socket* socket, io_Context* ctx)
{
    io_Descriptor_init(&socket->base, ctx);
}

IO_INLINE(io_Err)
io_Socket_read(io_Socket* socket, void* addr, size_t* size)
{
    return io_perform_read(&socket->base, addr, size);
}

IO_INLINE(io_Err)
io_Socket_write(io_Socket* socket, const void* addr, size_t* size)
{
    return io_perform_write(&socket->base, addr, size);
}

IO_INLINE(io_Err)
io_Socket_async_read(io_Socket* socket, void* addr, size_t size, io_ReadCallback callback, void* user_data)
{
    io_ReadOp* op = io_ReadOp_create(&socket->base, addr, size, callback, user_data);
    if (!op)
        return io_SystemErr(IO_ENOMEM);
    io_Handle_submit(socket->base.handle, &op->base);
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_Socket_async_write(io_Socket* socket, const void* addr, size_t size, io_WriteCallback callback, void* user_data)
{
    io_WriteOp* op = io_WriteOp_create(&socket->base, addr, size, callback, user_data);
    if (!op)
        return io_SystemErr(IO_ENOMEM);
    io_Handle_submit(socket->base.handle, &op->base);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Socket_deinit(io_Socket* socket)
{
    io_Descriptor_close(&socket->base);
}

#define DEFINE_SOCKET_WRAPPERS(P, B)                                                                      \
    IO_INLINE(io_Err)                                                                                     \
    P##_read(P* socket, void* addr, size_t* size)                                                         \
    {                                                                                                     \
        return B##_read(&socket->base, addr, size);                                                       \
    }                                                                                                     \
                                                                                                          \
    IO_INLINE(io_Err)                                                                                     \
    P##_async_read(P* socket, void* addr, size_t size, io_ReadCallback callback, void* user_data)         \
    {                                                                                                     \
        return B##_async_read(&socket->base, addr, size, callback, user_data);                            \
    }                                                                                                     \
                                                                                                          \
    IO_INLINE(io_Err)                                                                                     \
    P##_write(P* socket, const void* addr, size_t* size)                                                  \
    {                                                                                                     \
        return B##_write(&socket->base, addr, size);                                                      \
    }                                                                                                     \
                                                                                                          \
    IO_INLINE(io_Err)                                                                                     \
    P##_async_write(P* socket, const void* addr, size_t size, io_WriteCallback callback, void* user_data) \
    {                                                                                                     \
        return B##_async_write(&socket->base, addr, size, callback, user_data);                           \
    }

#endif
