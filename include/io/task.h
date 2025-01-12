#ifndef IO_TASK_H
#define IO_TASK_H

#include <stddef.h>

#include <io/err.h>

typedef void (*io_Task_fn)(void* self);

typedef struct io_Task {
    io_Task_fn fn;
    struct io_Task* next;
} io_Task;

typedef enum io_OpType {
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_MAX,
} io_OpType;

typedef enum io_OpFlags {
    IO_OP_COMPLETED = 1,
    IO_OP_TRYIO = 1 << 1,
} io_OpFlags;

typedef void (*io_Op_abort_fn)(void* self, io_Err err);

typedef struct io_Op {
    io_Task base;
    io_Op_abort_fn abort;
    io_OpType type;
    io_OpFlags flags;
} io_Op;

IO_INLINE(void)
io_Op_init(io_Op* task, io_OpType type, io_Task_fn fn, io_Op_abort_fn abort)
{
    task->base.fn = fn;
    task->type = type;
    task->abort = abort;
    task->flags = 0;
}

IO_INLINE(void)
io_Op_perform(io_Op* op)
{
    op->base.fn(op);
}

IO_INLINE(void)
io_Op_abort(io_Op* op, io_Err err)
{
    op->abort(op, err);
}

IO_INLINE(void)
io_Op_set_flags(io_Op* op, io_OpFlags flags)
{
    op->flags |= flags;
}

IO_INLINE(void)
io_Op_clear_flags(io_Op* op, io_OpFlags flags)
{
    op->flags &= ~flags;
}

IO_INLINE(io_OpFlags)
io_Op_flags(io_Op* op)
{
    return op->flags;
}

typedef struct io_IoHandler {
    io_Task base;
    void (*func)(void* self, size_t size, io_Err err);
} io_IoHandler;

#endif
