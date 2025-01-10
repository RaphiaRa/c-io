#ifndef IO_SOCKET_H
#define IO_SOCKET_H

#include <io/config.h>
#include <io/descriptor.h>
#include <io/read.h>
#include <io/task.h>

#include <stddef.h>

typedef struct io_WriteHandler {
    void (*fn)(void* self, size_t size, io_Err err);
    void (*destroy)(void* self);
} io_WriteHandler;

IO_INLINE(void)
io_WriteHandler_complete(io_WriteHandler* handler, size_t size, io_Err err)
{
    handler->fn(handler, size, err);
    handler->destroy(handler);
}

typedef struct io_Socket {
    io_Descriptor base;
} io_Socket;

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
    return io_read_perform(&socket->base, addr, size);
}

IO_INLINE(io_Err)
io_Socket_write(io_Socket* socket, const void* buf, size_t* size)
{
    (void)socket;
    (void)buf;
    (void)size;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Socket_async_read(io_Socket* socket, void* addr, size_t size, io_ReadHandler* handler)
{
    io_Handle_submit(socket->base.handle, &io_ReadOp_create(&socket->base, addr, size, handler)->base);
}

IO_INLINE(void)
io_Socket_async_write(io_Socket* socket, const void* buf, size_t size, io_WriteHandler* ch)
{
    (void)socket;
    (void)buf;
    (void)size;
    (void)ch;
}

IO_INLINE(void)
io_Socket_destroy(io_Socket* socket)
{
    io_Descriptor_close(&socket->base);
}

DEFINE_DESCRIPTOR_WRAPPERS(io_Socket, io_Descriptor)

#define DEFINE_SOCKET_WRAPPERS(P, B)                \
    IO_INLINE(io_Err)                               \
    P##_read(P* socket, void* addr, size_t* size)   \
    {                                               \
        return B##_read(&socket->base, addr, size); \
    }

#endif
