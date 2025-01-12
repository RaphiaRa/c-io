#ifndef IO_READ_H
#define IO_READ_H

#include <io/config.h>

#include <io/assert.h>
#include <io/context.h>
#include <io/descriptor.h>
#include <io/system_err.h>
#include <io/task.h>

#include <stdbool.h>

#include <sys/socket.h>

typedef void (*io_ReadCallback)(void* user_data, size_t size, io_Err err);

typedef struct io_ReadOp {
    io_Op base;
    io_Descriptor* socket;
    io_ReadCallback callback;
    void* addr;
    void* user_data;
    size_t size;
    size_t refcount;
    io_Err err;
} io_ReadOp;

IO_INLINE(io_Err)
io_perform_read(io_Descriptor* socket, void* addr, size_t* size)
{
    int read_fd = io_Descriptor_get_fd(socket);
    ssize_t ret = read(read_fd, addr, *size);
    if (ret == -1) {
        return io_SystemErr_make(errno);
    }
    *size = (size_t)ret;
    io_Descriptor_set_fd(socket, ret);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_ReadOp_complete(io_ReadOp* op, size_t size, io_Err err)
{
    op->err = err;
    op->size = size;
    io_Op_set_flags(&op->base, IO_OP_COMPLETED);
    ++op->refcount;
    io_Context_post(io_Descriptor_get_context(op->socket), &op->base.base);
}

IO_INLINE(void)
io_ReadOp_perform(io_ReadOp* op)
{
    size_t size = op->size;
    io_Err err = io_perform_read(op->socket, op->addr, &size);
    if (!io_ok(err)
        && (io_Op_flags(&op->base) & IO_OP_TRYIO)
        && err.category == io_SystemErrCategory()
        && (err.code == IO_EAGAIN || err.code == IO_EAGAIN)) {
        return;
    }
    io_ReadOp_complete(op, size, err);
}

IO_INLINE(void)
io_ReadOp_fn(void* self)
{
    io_ReadOp* op = self;
    if (io_Op_flags(&op->base) & IO_OP_COMPLETED) {
        op->callback(op->user_data, op->size, op->err);
    } else {
        io_ReadOp_perform(op);
    }
}

IO_INLINE(void)
io_ReadOp_abort(void* self, io_Err err)
{
    io_ReadOp* task = self;
    io_ReadOp_complete(task, 0, err);
}

IO_INLINE(void)
io_ReadOp_destroy(void* self)
{
    io_ReadOp* op = self;
    if (!(--op->refcount)) {
        io_Allocator_free(io_Descriptor_get_context(op->socket)->allocator, op);
    }
}

IO_INLINE(io_ReadOp*)
io_ReadOp_create(io_Descriptor* socket, void* addr, size_t size, io_ReadCallback callback, void* user_data)
{
    io_ReadOp* op = io_Allocator_alloc(io_Descriptor_get_context(socket)->allocator, sizeof(io_ReadOp));
    IO_REQUIRE(op, "Out of memory");
    io_Op_init(&op->base, IO_OP_READ, io_ReadOp_fn, io_ReadOp_abort, io_ReadOp_destroy);
    op->socket = socket;
    op->addr = addr;
    op->size = size;
    op->callback = callback;
    op->user_data = user_data;
    op->refcount = 1;
    return op;
}

#endif
