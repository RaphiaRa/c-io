/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_WRITE_H
#define IO_WRITE_H

#include <io/config.h>

#include <io/assert.h>
#include <io/context.h>
#include <io/descriptor.h>
#include <io/system_err.h>
#include <io/task.h>

#include <stdbool.h>

#include <sys/socket.h>

typedef void (*io_WriteCallback)(void* user_data, size_t size, io_Err err);

typedef struct io_WriteOp {
    io_Op base;
    io_Descriptor* socket;
    io_WriteCallback callback;
    const void* addr;
    void* user_data;
    size_t size;
    io_Err err;
} io_WriteOp;

IO_INLINE(io_Err)
io_perform_write(io_Descriptor* socket, const void* addr, size_t* size)
{
    int write_fd = io_Descriptor_get_fd(socket);
    ssize_t ret = write(write_fd, addr, *size);
    if (ret == -1) {
        return io_SystemErr_make(errno);
    }
    *size = (size_t)ret;
    io_Descriptor_set_fd(socket, ret);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_WriteOp_finalize(io_WriteOp* op)
{
    io_Allocator* allocator = io_Descriptor_get_context(op->socket)->allocator;
    op->callback(op->user_data, op->size, op->err);
    io_Allocator_free(allocator, op);
}

IO_INLINE(void)
io_WriteOp_complete(io_WriteOp* op, size_t size, io_Err err)
{
    op->err = err;
    op->size = size;
    io_Op_set_flags(&op->base, IO_OP_COMPLETED);
    io_Context_post(io_Descriptor_get_context(op->socket), &op->base.base);
}

IO_INLINE(void)
io_WriteOp_perform(io_WriteOp* op)
{
    size_t size = op->size;
    io_Err err = io_perform_write(op->socket, op->addr, &size);
    if (!io_ok(err)
        && (io_Op_flags(&op->base) & IO_OP_TRYIO)
        && err.category == io_SystemErrCategory()
        && (err.code == IO_EAGAIN || err.code == IO_EAGAIN)) {
        return;
    }
    io_WriteOp_complete(op, size, err);
}

IO_INLINE(void)
io_WriteOp_fn(void* self)
{
    io_WriteOp* op = self;
    if (io_Op_flags(&op->base) & IO_OP_COMPLETED) {
        io_WriteOp_finalize(op);
    } else {
        io_WriteOp_perform(op);
    }
}

IO_INLINE(void)
io_WriteOp_abort(void* self, io_Err err)
{
    io_WriteOp* task = self;
    io_WriteOp_complete(task, 0, err);
}

IO_INLINE(io_WriteOp*)
io_WriteOp_create(io_Descriptor* socket, const void* addr, size_t size, io_WriteCallback callback, void* user_data)
{
    io_WriteOp* op = io_Allocator_alloc(io_Descriptor_get_context(socket)->allocator, sizeof(io_WriteOp));
    if (!op)
        return NULL;
    io_Op_init(&op->base, IO_OP_WRITE, io_WriteOp_fn, io_WriteOp_abort);
    op->socket = socket;
    op->addr = addr;
    op->size = size;
    op->callback = callback;
    op->user_data = user_data;
    return op;
}

#endif
