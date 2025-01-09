#ifndef IO_SOCKET_H
#define IO_SOCKET_H

#include <io/io_config.h>
#include <io/io_task.h>

#include <stddef.h>

typedef struct io_ReadHandler {
    void (*fn)(void* self, size_t size, io_Err err);
    void (*destroy)(void* self);
} io_ReadHandler;

IO_INLINE(void)
io_ReadHandler_complete(io_ReadHandler* handler, size_t size, io_Err err)
{
    handler->fn(handler, size, err);
    handler->destroy(handler);
}

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

typedef struct io_SocketMethods {
    io_Err (*read)(void* self, void* buf, size_t size);
    io_Err (*write)(void* self, const void* buf, size_t size);
    void (*async_read)(void* self, void* buf, size_t size, io_ReadHandler* ch);
    void (*async_write)(void* self, const void* buf, size_t size, io_WriteHandler* ch);
    void (*set_fd)(void* self, int fd);
    int (*get_fd)(void* self);
    io_Context* (*get_context)(void* self);
} io_SocketMethods;

typedef struct io_Socket {
    const io_SocketMethods* methods;
} io_Socket;

IO_INLINE(io_Err)
io_Socket_read(io_Socket* socket, void* buf, size_t size)
{
    return socket->methods->read(socket, buf, size);
}

IO_INLINE(io_Err)
io_Socket_write(io_Socket* socket, const void* buf, size_t size)
{
    return socket->methods->write(socket, buf, size);
}

IO_INLINE(void)
io_Socket_async_read(io_Socket* socket, void* buf, size_t size, io_ReadHandler* ch)
{
    socket->methods->async_read(socket, buf, size, ch);
}

IO_INLINE(void)
io_Socket_async_write(io_Socket* socket, const void* buf, size_t size, io_WriteHandler* ch)
{
    socket->methods->async_write(socket, buf, size, ch);
}

IO_INLINE(void)
io_Socket_set_fd(io_Socket* socket, int fd)
{
    socket->methods->set_fd(socket, fd);
}

IO_INLINE(int)
io_Socket_get_fd(io_Socket* socket)
{
    return socket->methods->get_fd(socket);
}

IO_INLINE(io_Context*)
io_Socket_get_context(io_Socket* socket)
{
    return socket->methods->get_context(socket);
}

#endif
