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
#include <io/thread.h>
#include <io/vec.h>

#define io_LoopVec_destroy_element(vec) io_Loop_destroy(*(vec))

IO_DEFINE_VEC(io_LoopVec, io_Loop*, io_LoopVec_destroy_element)
IO_DEFINE_VEC(io_ThreadVec, io_Thread, io_Thread_deinit)

typedef struct io_Context {
    io_LoopVec threadLoops;
    io_ThreadVec threads;
    io_ThisThreadData this_loop;
    io_Allocator* allocator;
    io_Loop* loop;
    size_t num_threads;
    size_t num_tasks;
    size_t round_robin_index;
} io_Context;

IO_INLINE(io_Err)
io_Context_init(io_Context* context, io_Allocator* allocator)
{
    context->allocator = allocator ? allocator : io_SystemAllocator();
    context->num_threads = 0;
    context->num_tasks = 0;
    context->round_robin_index = 0;
    io_LoopVec_init(&context->threadLoops, context->allocator);
    io_ThreadVec_init(&context->threads, context->allocator);
    io_Err err = IO_ERR_OK;
    io_Loop* loop;
    if ((err = io_Loop_create(&loop, &context->num_tasks, context->allocator))) {
        return err;
    }
    io_Reactor* reactor = NULL;
    if ((err = io_Poll_create(&reactor, loop, context->allocator))) {
        io_Loop_destroy(loop);
        return err;
    }
    io_Loop_set_reactor(loop, reactor);
    if ((err = io_ThisThreadData_init(&context->this_loop))) {
        goto destroy_reactor;
    }
    io_ThisThreadData_set(&context->this_loop, loop);
    context->loop = loop;
    return IO_ERR_OK;
destroy_reactor:
    io_Reactor_destroy(reactor);
    return err;
}

IO_INLINE(io_Err)
io_Context_set_num_threads(io_Context* context, size_t num_threads)
{
    io_Err err = IO_ERR_OK;
    for (size_t i = 0; i < num_threads; i++) {
        io_Loop* loop;
        if ((err = io_Loop_create(&loop, &context->num_tasks, context->allocator))) {
            goto reset_loop_clear;
        }
        io_Reactor* reactor = NULL;
        if ((err = io_Poll_create(&reactor, loop, context->allocator))) {
            io_Loop_destroy(loop);
            goto reset_loop_clear;
        }
        io_Loop_set_reactor(loop, reactor);
        io_LoopVec_push_back(&context->threadLoops, loop);
    }
    context->num_threads = num_threads;
    return IO_ERR_OK;
reset_loop_clear:
    io_LoopVec_clear(&context->threadLoops);
    return err;
}

IO_INLINE(void)
io_Context_deinit(io_Context* context)
{
    io_Loop_destroy(context->loop);
}

typedef struct io_ContextThreadData {
    io_Context* context;
    size_t index;
} io_ContextThreadData;

IO_INLINE(void*)
io_Context_run_thread(void* user_data)
{
    io_ContextThreadData* data = user_data;
    io_Loop* loop = *io_LoopVec_at(&data->context->threadLoops, data->index);
    io_ThisThreadData_set(&data->context->this_loop, loop);
    io_Loop_run(loop);
    io_free(data->context->allocator, data);
    return NULL;
}

IO_INLINE(io_Err)
io_Context_run_threads(io_Context* context)
{
    io_Err err = IO_ERR_OK;
    for (size_t i = 0; i < context->num_threads; i++) {
        io_ContextThreadData* data = io_alloc(context->allocator, sizeof(io_ContextThreadData));
        if (!data) {
            err = io_SystemErr(IO_ENOMEM);
            goto clear_thread_vec;
        }
        data->context = context;
        data->index = i;
        io_Thread thread;
        if ((err = io_Thread_init(&thread, io_Context_run_thread, data))) {
            io_free(context->allocator, data);
            goto clear_thread_vec;
        }
        io_ThreadVec_push_back(&context->threads, thread);
    }
    return IO_ERR_OK;
clear_thread_vec:
    io_ThreadVec_clear(&context->threads);
    return err;
}

IO_INLINE(io_Err)
io_Context_run(io_Context* context)
{
    io_Err err = IO_ERR_OK;
    if ((err = io_Context_run_threads(context))) {
        return err;
    }
    io_Loop_run(context->loop);
    return IO_ERR_OK;
}

IO_INLINE(io_Loop*)
io_Context_this_loop(io_Context* context)
{
    io_Loop* loop = io_ThisThreadData_get(&context->this_loop);
    if (!loop) {
        loop = context->loop;
    }
    return loop;
}

IO_INLINE(io_Loop*)
io_Context_next_loop(io_Context* context)
{
    size_t index = io_atomic_fetch_add(&context->round_robin_index, 1);
    index %= (context->num_threads + 1);
    switch (index) {
    case 0:
        return context->loop;
        break;
    default:
        return *io_LoopVec_at(&context->threadLoops, index - 1);
        break;
    }
}

IO_INLINE(void)
io_Context_post(io_Context* context, io_Task* task)
{
    io_Loop_push_task(io_Context_this_loop(context), task);
}

IO_INLINE(io_Allocator*)
io_Context_allocator(io_Context* context)
{
    return context->allocator;
}

#endif
