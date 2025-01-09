#ifndef IO_ACCEPTOR_H
#define IO_ACCEPTOR_H

#include <io/io_config.h>
#include <io/io_context.h>
#include <io/io_err.h>
#include <io/io_socket.h>
#include <io/io_task.h>

typedef struct io_AcceptHandler {
    void (*fn)(void* self, io_Err err);
    void (*destroy)(void* self);
} io_AcceptHandler;

IO_INLINE(void)
io_AcceptHandler_complete(io_AcceptHandler* handler, io_Err err)
{
    handler->fn(handler, err);
    handler->destroy(handler);
}

typedef struct io_AcceptorMethods {
    void (*async_accept)(void* self, io_Socket* socket, io_AcceptHandler* ch);
    io_Err (*accept)(void* self, io_Socket* socket);
    int (*get_fd)(const void* self);
    io_Context* (*get_context)(void* self);
} io_AcceptorMethods;

typedef struct io_Acceptor {
    const io_AcceptorMethods* methods;
} io_Acceptor;

IO_INLINE(void)
io_Acceptor_async_accept(io_Acceptor* acceptor, io_Socket* socket, io_AcceptHandler* ch)
{
    acceptor->methods->async_accept(acceptor, socket, ch);
}

IO_INLINE(io_Err)
io_Acceptor_accept(io_Acceptor* acceptor, io_Socket* socket)
{
    return acceptor->methods->accept(acceptor, socket);
}

IO_INLINE(int)
io_Acceptor_get_fd(const io_Acceptor* acceptor)
{
    return acceptor->methods->get_fd(acceptor);
}

IO_INLINE(io_Context*)
io_Acceptor_get_context(io_Acceptor* acceptor)
{
    return acceptor->methods->get_context(acceptor);
}

#define io_accept(acceptor, socket) io_Acceptor_accept((io_Acceptor*)acceptor, (io_Socket*)socket)
#define io_async_accept(acceptor, socket, handler) io_Acceptor_accept((io_Acceptor*)acceptor, (io_Socket*)socket, (io_AcceptHandler*)handler)

#endif
