#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <stddef.h>

#include <io/io_allocator.h>
#include <io/io_assert.h>
#include <io/io_loop.h>
#include <io/io_poll.h>
#include <io/io_queue.h>

typedef struct io_Context {
    io_Allocator* allocator;
    io_Loop loop;
} io_Context;

IO_INLINE(io_Context)
io_Context_make(void)
{
    io_Context context;
    context.allocator = io_DefaultAllocator();
    io_Loop_init(&context.loop, io_Poll_create(&context.loop));
    return context;
}

IO_INLINE(void)
io_Context_post(io_Context* context, io_Task* task)
{
    io_Loop_push_task(&context->loop, task);
}

#endif
