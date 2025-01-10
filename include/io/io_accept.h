#ifndef IO_ACCEPT_H
#define IO_ACCEPT_H

#include <io/io_config.h>

#include <io/io_assert.h>
#include <io/io_context.h>
#include <io/io_descriptor.h>
#include <io/io_system_err.h>
#include <io/io_task.h>

#include <stdbool.h>

#include <sys/socket.h>

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

typedef struct io_AcceptOp {
    io_Op base;
    io_AcceptHandler* handler;
    io_Descriptor* acceptor;
    io_Descriptor* socket;
} io_AcceptOp;

IO_INLINE(io_Err)
io_accept_perform(const io_Descriptor* acceptor, io_Descriptor* socket)
{
    int accept_fd = io_Descriptor_get_fd(acceptor);
    int ret = accept(accept_fd, NULL, NULL);
    if (ret == -1) {
        return io_SystemErr_make(errno);
    }
    io_Descriptor_set_fd(socket, ret);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_AcceptOp_fn(void* self)
{
    io_AcceptOp* task = self;
    int fd = 0;
    io_Err err = io_accept_perform(task->acceptor, task->socket);
    if (!IO_ERR_HAS(err)) {
        io_Op_set_completed(&task->base, true);
    }

    io_AcceptHandler_complete(task->handler, err);
}

IO_INLINE(void)
io_AcceptOp_destroy(void* self)
{
    io_AcceptOp* task = self;
    io_Allocator_free(io_Descriptor_get_context(task->acceptor)->allocator, task);
}

IO_INLINE(io_AcceptOp*)
io_AcceptOp_create(io_Descriptor* acceptor, io_Descriptor* socket, io_AcceptHandler* handler)
{
    io_AcceptOp* task = io_Allocator_alloc(io_Descriptor_get_context(acceptor)->allocator, sizeof(io_AcceptOp));
    IO_REQUIRE(task, "Out of memory");
    io_Op_init(&task->base, IO_OP_READ, io_AcceptOp_fn, io_AcceptOp_destroy);
    task->acceptor = acceptor;
    task->socket = socket;
    task->handler = handler;
    return task;
}

#endif
