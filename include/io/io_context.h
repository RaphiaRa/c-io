#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <stddef.h>

#include <io/io_allocator.h>
#include <io/io_queue.h>
#include <io/io_reactor.h>
#include <io/io_task.h>

IO_DEFINE_QUEUE(io_TaskQueue, io_Task)

typedef struct io_Loop {
    size_t num_tasks;
    io_TaskQueue task_queue;
    io_Reactor* reactor;
    io_Task reactor_task;
} io_Loop;

typedef struct io_Context {
    struct io_Loop* loop;
    io_Allocator* allocator;
} io_Context;

IO_INLINE(io_Err)
io_Context_init(io_Context* context, io_Allocator* allocator)
{
    (void)context;
    (void)allocator;
    return IO_ERR_OK;
}


#endif
