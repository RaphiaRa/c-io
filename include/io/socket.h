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

IO_INLINE(io_Socket)
io_Socket_make(io_Context* context)
{
    io_Socket socket = {
        .base = io_Descriptor_make(context),
    };
    return socket;
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

IO_INLINE(void)
io_Socket_async_read(io_Socket* socket, void* addr, size_t size, io_ReadCallback callback, void* user_data)
{
    io_Handle_submit(socket->base.handle, &io_ReadOp_create(&socket->base, addr, size, callback, user_data)->base);
}

IO_INLINE(void)
io_Socket_async_write(io_Socket* socket, const void* addr, size_t size, io_WriteCallback callback, void* user_data)
{
    io_Handle_submit(socket->base.handle, &io_WriteOp_create(&socket->base, addr, size, callback, user_data)->base);
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
    IO_INLINE(void)                                                                                       \
    P##_async_read(P* socket, void* addr, size_t size, io_ReadCallback callback, void* user_data)         \
    {                                                                                                     \
        B##_async_read(&socket->base, addr, size, callback, user_data);                  \
    }                                                                                                     \
                                                                                                          \
    IO_INLINE(io_Err)                                                                                     \
    P##_write(P* socket, const void* addr, size_t* size)                                                  \
    {                                                                                                     \
        return B##_write(&socket->base, addr, size);                                                      \
    }                                                                                                     \
                                                                                                          \
    IO_INLINE(void)                                                                                       \
    P##_async_write(P* socket, const void* addr, size_t size, io_WriteCallback callback, void* user_data) \
    {                                                                                                     \
        B##_async_write(&socket->base, addr, size, callback, user_data);                 \
    }

#endif
