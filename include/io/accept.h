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

typedef void (*io_AcceptCallback)(void* user_data, io_Err err);

typedef struct io_AcceptOp {
    io_Op base;
    io_Descriptor* acceptor;
    io_Descriptor* socket;
    io_AcceptCallback callback;
    void* user_data;
    io_Err err;
} io_AcceptOp;

IO_INLINE(io_Err)
io_perform_accept(const io_Descriptor* acceptor, io_Descriptor* socket)
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
io_AcceptOp_finalize(io_AcceptOp* op)
{
    io_Allocator* allocator = io_Descriptor_get_context(op->acceptor)->allocator;
    op->callback(op->user_data, op->err);
    io_Allocator_free(allocator, op);
}

IO_INLINE(void)
io_AcceptOp_complete(io_AcceptOp* op, io_Err err)
{
    op->err = err;
    io_Op_set_flags(&op->base, IO_OP_COMPLETED);
    io_Context_post(io_Descriptor_get_context(op->acceptor), &op->base.base);
}

IO_INLINE(void)
io_AcceptOp_perform(io_AcceptOp* op)
{
    io_Err err = io_perform_accept(op->acceptor, op->socket);
    if (!io_ok(err)
        && (io_Op_flags(&op->base) & IO_OP_TRYIO)
        && err.category == io_SystemErrCategory()
        && (err.code == IO_EAGAIN || err.code == IO_EAGAIN)) {
        return;
    }
    io_AcceptOp_complete(op, err);
}

IO_INLINE(void)
io_AcceptOp_fn(void* self)
{
    io_AcceptOp* op = self;
    if (io_Op_flags(&op->base) & IO_OP_COMPLETED) {
        io_AcceptOp_finalize(op);
    } else {
        io_AcceptOp_perform(op);
    }
}

IO_INLINE(void)
io_AcceptOp_abort(void* self, io_Err err)
{
    io_AcceptOp* task = self;
    io_AcceptOp_complete(task, err);
}

IO_INLINE(io_AcceptOp*)
io_AcceptOp_create(io_Descriptor* acceptor, io_Descriptor* socket, io_AcceptCallback callback, void* user_data)
{
    io_AcceptOp* task = io_Allocator_alloc(io_Descriptor_get_context(acceptor)->allocator, sizeof(io_AcceptOp));
    IO_REQUIRE(task, "Out of memory");
    io_Op_init(&task->base, IO_OP_READ, io_AcceptOp_fn, io_AcceptOp_abort);
    task->acceptor = acceptor;
    task->socket = socket;
    task->callback = callback;
    task->user_data = user_data;
    return task;
}

#endif
