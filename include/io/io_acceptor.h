#ifndef IO_ACCEPTOR_H
#define IO_ACCEPTOR_H

#include <io/io_accept.h>
#include <io/io_config.h>
#include <io/io_context.h>
#include <io/io_descriptor.h>
#include <io/io_err.h>
#include <io/io_socket.h>
#include <io/io_task.h>

typedef struct io_Acceptor {
    io_Descriptor base;
} io_Acceptor;

IO_INLINE(void)
io_Acceptor_init(io_Acceptor* acceptor, io_Context* context)
{
    io_Descriptor_init(&acceptor->base, context);
}

IO_INLINE(void)
io_Acceptor_async_accept(io_Acceptor* acceptor, io_Socket* socket, io_AcceptHandler* ch)
{
    io_Handle_submit(acceptor->base.handle, &io_AcceptOp_create(&acceptor->base, &socket->base, ch)->base);
}

IO_INLINE(io_Err)
io_Acceptor_accept(io_Acceptor* acceptor, io_Socket* socket)
{
    return io_accept_perform(&acceptor->base, &socket->base);
}

DEFINE_DESCRIPTOR_WRAPPERS(io_Acceptor, io_Descriptor)

#define DEFINE_ACCEPT_WRAPPERS(A, S)                                              \
    IO_INLINE(void)                                                               \
    A##_async_accept(A* acceptor, S* socket, io_AcceptHandler* handler)           \
    {                                                                             \
        return io_Acceptor_async_accept(&acceptor->base, &socket->base, handler); \
    }                                                                             \
                                                                                  \
    IO_INLINE(io_Err)                                                             \
    A##_accept(A* acceptor, S* socket)                                            \
    {                                                                             \
        return io_Acceptor_accept(&acceptor->base, &socket->base);                \
    }

#endif
