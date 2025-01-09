#ifndef IO_BASIC_ACCEPTOR_H
#define IO_BASIC_ACCEPTOR_H

#include <io/io_accept.h>
#include <io/io_assert.h>
#include <io/io_config.h>
#include <io/io_context.h>
#include <io/io_reactor.h>

typedef struct io_BasicAcceptor {
    io_Acceptor base;
    io_Context* context;
    io_Handle* handle;
} io_BasicAcceptor;


IO_INLINE(void)
io_BasicAcceptor_async_accept(void* self, io_Socket* socket, io_AcceptHandler* ch)
{
    io_BasicAcceptor* acceptor = self;
    io_Handle_submit(acceptor->handle, &io_AcceptOp_create(&acceptor->base, socket, ch)->base);
}

IO_INLINE(io_Err)
io_BasicAcceptor_accept(void* self, io_Socket* socket)
{
    io_BasicAcceptor* acceptor = self;
    return io_accept_perform(&acceptor->base, socket);
}

IO_INLINE(int)
io_BasicAcceptor_get_fd(const void* self)
{
    const io_BasicAcceptor* acceptor = self;
    return io_Handle_get_fd(acceptor->handle);
}

IO_INLINE(io_Context*)
io_BasicAcceptor_get_context(void* self)
{
    io_BasicAcceptor* acceptor = self;
    return acceptor->context;
}

IO_INLINE(void)
io_BasicAcceptor_init(io_BasicAcceptor* acceptor, io_Context* context)
{
    static io_AcceptorMethods methods = {
        .async_accept = io_BasicAcceptor_async_accept,
        .accept = io_BasicAcceptor_accept,
        .get_fd = io_BasicAcceptor_get_fd,
        .get_context = io_BasicAcceptor_get_context,
    };
    acceptor->base.methods = &methods;
    acceptor->context = context;
    acceptor->handle = NULL;
}

IO_INLINE(void)
io_BasicAcceptor_set_fd(io_BasicAcceptor* acceptor, int fd);

IO_INLINE(void)
io_BasicAcceptor_clear_fd(io_BasicAcceptor* acceptor);

IO_INLINE(void)
io_BasicAcceptor_set_fd(io_BasicAcceptor* acceptor, int fd)
{
    if (acceptor->handle) {
        io_BasicAcceptor_clear_fd(acceptor);
    }
    acceptor->handle = io_Reactor_create_handle(acceptor->context->loop->reactor, fd);
}

IO_INLINE(void)
io_BasicAcceptor_clear_fd(io_BasicAcceptor* acceptor)
{
    if (acceptor->handle) {
        io_Handle_destroy(acceptor->handle);
        acceptor->handle = NULL;
    }
}

#endif
