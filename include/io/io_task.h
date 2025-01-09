#ifndef IO_TASK_H
#define IO_TASK_H

#include <stddef.h>

#include <io/io_err.h>

typedef void (*io_Task_fn)(void* self);
typedef void (*io_Task_destroy)(void* self);

typedef struct io_Task {
    io_Task_fn fn;
    io_Task_destroy destroy;
    struct io_Task* next;
} io_Task;

typedef enum io_OpType {
    IO_OP_READ,
    IO_OP_WRITE,
    IO_OP_MAX,
} io_OpType;

typedef struct io_Op {
    io_Task base;
    io_OpType io_type;
    bool completed;
} io_Op;

IO_INLINE(void)
io_Op_init(io_Op* task, io_OpType type, io_Task_fn fn, io_Task_destroy destroy)
{
    task->base.fn = fn;
    task->base.destroy = destroy;
    task->io_type = type;
    task->completed = false;
}

IO_INLINE(void)
io_Op_set_completed(io_Op* op, bool completed)
{
    op->completed = completed;
}

typedef struct io_IoHandler {
    io_Task base;
    void (*func)(void* self, size_t size, io_Err err);
} io_IoHandler;

#endif
