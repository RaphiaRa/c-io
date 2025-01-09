#ifndef IO_READ_H
#define IO_READ_H

#include <io/io_config.h>

#include <io/io_assert.h>
#include <io/io_context.h>
#include <io/io_socket.h>
#include <io/io_system_err.h>
#include <io/io_task.h>

#include <stdbool.h>

#include <sys/socket.h>
#include <unistd.h>

typedef struct io_ReadOp {
    io_Op base;
    io_ReadHandler* handler;
    io_Socket* socket;
    void* addr;
    size_t len;
} io_ReadOp;

IO_INLINE(io_Err)
io_read_perform(io_Socket* socket, void* addr, size_t* size)
{
    ssize_t ret = read(io_Socket_get_fd(socket), addr, *size);
    if (ret == -1) {
        return io_SystemErr_make(errno);
    }
    *size = (size_t)ret;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_ReadOp_fn(void* self)
{
    io_ReadOp* task = self;
    size_t bytes = task->len;
    io_Err err = io_read_perform(task->socket, task->addr, &bytes);
    if (!IO_ERR_HAS(err)) {
        io_Op_set_completed(&task->base, true);
    }
    io_ReadHandler_complete(task->handler, bytes, err);
}

IO_INLINE(void)
io_ReadOp_destroy(void* self)
{
    io_ReadOp* task = self;
    io_Allocator_free(io_Socket_get_context(task->socket)->allocator, task);
}

IO_INLINE(io_ReadOp*)
io_ReadOp_create(io_Socket* socket, void* addr, size_t len, io_ReadHandler* handler)
{
    io_ReadOp* task = io_Allocator_alloc(io_Socket_get_context(socket)->allocator, sizeof(io_ReadOp));
    IO_REQUIRE(task, "Out of memory");
    io_Op_init(&task->base, IO_OP_READ, io_ReadOp_fn, io_ReadOp_destroy);
    task->socket = socket;
    task->addr = addr;
    task->len = len;
    task->handler = handler;
    return task;
}

#endif
