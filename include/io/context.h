/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <stddef.h>

#include <io/allocator.h>
#include <io/assert.h>
#include <io/loop.h>
#include <io/poll.h>
#include <io/queue.h>

typedef struct io_Context {
    io_Allocator* allocator;
    io_Loop* loop;
} io_Context;

IO_INLINE(io_Err)
io_Context_init(io_Context* context)
{
    context->allocator = io_DefaultAllocator();
    io_Loop* loop = io_Loop_create();
    if (loop == NULL) {
        return io_SystemErr(IO_ENOMEM);
    }
    io_Reactor* reactor = io_Poll_create(loop);
    if (reactor == NULL) {
        io_Reactor_destroy(reactor);
        return io_SystemErr(IO_ENOMEM);
    }
    io_Loop_set_reactor(loop, reactor);
    context->loop = loop;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Context_deinit(io_Context* context)
{
    io_Loop_destroy(context->loop);
}

IO_INLINE(void)
io_Context_run(io_Context* context)
{
    io_Loop_run(context->loop);
}

IO_INLINE(void)
io_Context_post(io_Context* context, io_Task* task)
{
    io_Loop_push_task(context->loop, task);
}

IO_INLINE(io_Allocator*)
io_Context_allocator(io_Context* context)
{
    return context->allocator;
}

#endif
