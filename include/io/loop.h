/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_LOOP_H
#define IO_LOOP_H

#include <io/assert.h>
#include <io/atomic.h>
#include <io/err.h>
#include <io/queue.h>
#include <io/reactor.h>
#include <io/task.h>
#include <io/thread.h>

IO_DEFINE_QUEUE(io_TaskQueue, io_Task)

typedef struct io_Loop {
    io_TaskQueue queue;
    io_Task reactor_task;
    io_Mutex mutex;
    io_Reactor* reactor;
    io_Allocator* allocator;
    size_t* num_tasks;
    bool needs_interrupt;
} io_Loop;

IO_INLINE(io_Err)
io_Loop_create(io_Loop** out, size_t* task_counter, io_Allocator* allocator)
{
    io_Loop* loop = io_alloc(allocator, sizeof(io_Loop));
    if (!loop)
        return io_SystemErr(IO_ENOMEM);
    loop->num_tasks = task_counter;
    loop->queue = (io_TaskQueue){0};
    loop->reactor = NULL;
    loop->allocator = allocator;
    loop->needs_interrupt = false;
    io_Err err = IO_ERR_OK;
    if ((err = io_Mutex_init(&loop->mutex))) {
        goto free_loop;
    }
    io_TaskQueue_push(&loop->queue, &loop->reactor_task);
    *out = loop;
    return IO_ERR_OK;
free_loop:
    io_free(allocator, loop);
    return err;
}

/** io_Loop_set_reactor
 * @brief Set the reactor for the loop, the loop will take ownership
 * of the reactor and will destroy it when the loop is destroyed.
 */
IO_INLINE(void)
io_Loop_set_reactor(io_Loop* loop, io_Reactor* reactor)
{
    loop->reactor = reactor;
}

IO_INLINE(void)
io_Loop_decrease_task_count(io_Loop* loop)
{
    io_atomic_dec(loop->num_tasks);
}

IO_INLINE(void)
io_Loop_increase_task_count(io_Loop* loop)
{
    io_atomic_inc(loop->num_tasks);
    io_Mutex_lock(&loop->mutex);
    if (loop->needs_interrupt) {
        loop->needs_interrupt = false;
        io_Reactor_interrupt(loop->reactor);
    }
    io_Mutex_unlock(&loop->mutex);
}

IO_INLINE(size_t)
io_Loop_get_task_count(io_Loop* loop)
{
    return io_atomic_load(loop->num_tasks);
}

IO_INLINE(void)
io_Loop_push_task(io_Loop* loop, io_Task* task)
{
    io_Loop_increase_task_count(loop);
    io_Mutex_lock(&loop->mutex);
    io_TaskQueue_push(&loop->queue, task);
    if (loop->needs_interrupt) {
        loop->needs_interrupt = false;
        io_Reactor_interrupt(loop->reactor);
    }
    io_Mutex_unlock(&loop->mutex);
}

IO_INLINE(void)
io_Loop_run(io_Loop* loop)
{
    IO_REQUIRE(loop->reactor, "Reactor must be set before running the loop");
    while (io_Loop_get_task_count(loop) > 0) {
        while (1) {
            io_Mutex_lock(&loop->mutex);
            io_Task* task = io_TaskQueue_pop(&loop->queue);
            IO_ASSERT(task, "Task queue must never be empty");
            bool empty = io_TaskQueue_empty(&loop->queue);
            loop->needs_interrupt = empty;
            io_Mutex_unlock(&loop->mutex);
            if (task == &loop->reactor_task) {
                io_Reactor_run(loop->reactor, io_Seconds(empty ? -1 : 0));
                io_TaskQueue_push(&loop->queue, &loop->reactor_task);
            } else {
                task->fn(task);
                io_Loop_decrease_task_count(loop);
                break;
            }
        }
    }
}

IO_INLINE(void)
io_Loop_destroy(io_Loop* loop)
{
    io_Mutex_deinit(&loop->mutex);
    if (loop->reactor)
        io_Reactor_destroy(loop->reactor);
    io_free(loop->allocator, loop);
}

#endif
