#ifndef IO_READ_H
#define IO_READ_H

#include <io/io_config.h>

#include <io/io_assert.h>
#include <io/io_context.h>
#include <io/io_descriptor.h>
#include <io/io_system_err.h>
#include <io/io_task.h>

#include <stdbool.h>

#include <sys/socket.h>
#include <unistd.h>

typedef void (*io_ReadHandler_fn)(void* self, size_t size, io_Err err);

typedef struct io_ReadHandler {
    io_Task base;
    io_ReadHandler_fn fn;
    size_t size;
    io_Err err;
} io_ReadHandler;

IO_INLINE(void)
io_ReadHandler_perform(void* self)
{
    io_ReadHandler* handler = self;
    handler->fn(self, handler->size, handler->err);
}

IO_INLINE(void)
io_ReadHandler_set_args(io_ReadHandler* handler, size_t size, io_Err err)
{
    handler->size = size;
    handler->err = err;
}

typedef struct io_ReadOp {
    io_Op base;
    io_ReadHandler* handler;
    io_Descriptor* socket;
    void* addr;
    size_t len;
} io_ReadOp;

IO_INLINE(io_Err)
io_read_perform(io_Descriptor* socket, void* addr, size_t* size)
{
    ssize_t ret = read(io_Descriptor_get_fd(socket), addr, *size);
    if (ret == -1) {
        return io_SystemErr_make(errno);
    }
    *size = (size_t)ret;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_ReadOp_complete(io_ReadOp* op, size_t size, io_Err err)
{
    io_ReadHandler_set_args(op->handler, size, err);
    io_Context_post(io_Descriptor_get_context(op->socket), &op->handler->base);
}

IO_INLINE(void)
io_ReadOp_fn(void* self)
{
    io_ReadOp* task = self;
    size_t bytes = task->len;
    io_Err err = io_read_perform(task->socket, task->addr, &bytes);
    if (!IO_ERR_HAS(err)) {
        io_Op_set_flags(&task->base, IO_OP_COMPLETED);
    } else if ((io_Op_flags(&task->base) & IO_OP_TRYIO)
               && err.category == io_SystemErrCategory()
               && (err.code == IO_EAGAIN || err.code == IO_EAGAIN)) {
        return;
    }
    io_ReadOp_complete(task, bytes, err);
}

IO_INLINE(void)
io_ReadOp_destroy(void* self)
{
    io_ReadOp* task = self;
    io_Op_destroy(&task->base);
}

IO_INLINE(void)
io_ReadOp_abort(void* self, io_Err err)
{
    io_ReadOp* task = self;
    io_ReadOp_complete(task, 0, err);
}

IO_INLINE(io_ReadOp*)
io_ReadOp_create(io_Descriptor* socket, void* addr, size_t len, io_ReadHandler* handler)
{
    io_ReadOp* task = io_Allocator_alloc(io_Descriptor_get_context(socket)->allocator, sizeof(io_ReadOp));
    IO_REQUIRE(task, "Out of memory");
    io_Op_init(&task->base, IO_OP_READ, io_ReadOp_fn, io_ReadOp_abort, io_ReadOp_destroy);
    task->socket = socket;
    task->addr = addr;
    task->len = len;
    task->handler = handler;
    return task;
}

#endif
