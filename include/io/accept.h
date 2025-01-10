#ifndef IO_ACCEPT_H
#define IO_ACCEPT_H

#include <io/config.h>

#include <io/assert.h>
#include <io/context.h>
#include <io/descriptor.h>
#include <io/system_err.h>
#include <io/task.h>

#include <stdbool.h>

#include <sys/socket.h>

typedef void (*io_AcceptHandler_fn)(void* self, io_Err err);

typedef struct io_AcceptHandler {
    io_Task base;
    io_AcceptHandler_fn fn;
    io_Err err;
} io_AcceptHandler;

IO_INLINE(void)
io_AcceptHandler_perform(void* self)
{
    io_AcceptHandler* handler = self;
    handler->fn(self, handler->err);
}

IO_INLINE(void)
io_AcceptHandler_set_args(io_AcceptHandler* handler, io_Err err)
{
    handler->err = err;
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
io_AcceptOp_complete(io_AcceptOp* op, io_Err err)
{
    io_AcceptHandler_set_args(op->handler, err);
    io_Context_post(io_Descriptor_get_context(op->acceptor), &op->handler->base);
}

IO_INLINE(void)
io_AcceptOp_fn(void* self)
{
    io_AcceptOp* task = self;
    int fd = 0;
    io_Err err = io_accept_perform(task->acceptor, task->socket);
    if (!IO_ERR_HAS(err)) {
        io_Op_set_flags(&task->base, IO_OP_COMPLETED);
    } else if ((io_Op_flags(&task->base) & IO_OP_TRYIO)
               && err.category == io_SystemErrCategory()
               && (err.code == IO_EAGAIN || err.code == IO_EAGAIN)) {
        return;
    }
    io_AcceptOp_complete(task, err);
}

IO_INLINE(void)
io_AcceptOp_abort(void* self, io_Err err)
{
    io_AcceptOp* task = self;
    io_AcceptOp_complete(task, err);
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
    io_Op_init(&task->base, IO_OP_READ, io_AcceptOp_fn, io_AcceptOp_abort, io_AcceptOp_destroy);
    task->acceptor = acceptor;
    task->socket = socket;
    task->handler = handler;
    return task;
}

#endif
