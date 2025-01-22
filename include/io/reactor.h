/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_REACTOR_H
#define IO_REACTOR_H

#include <stdbool.h>

#include <io/config.h>
#include <io/err.h>
#include <io/task.h>
#include <io/timer.h>

#define IO_TIMEOUT_INFINITE ((io_Duration)(-1))

typedef struct io_HandleMethods {
    void (*cancel)(void* self);
    io_Err (*submit)(void* self, io_Op* op);
    void (*set_timeout)(void* self, io_OpType type, io_Duration duration);
    int (*get_fd)(const void* self);
    void (*destroy)(void* self);
} io_HandleMethods;

typedef struct io_Handle {
    const io_HandleMethods* methods;
} io_Handle;

IO_INLINE(void)
io_Handle_cancel(io_Handle* io_handle)
{
    io_handle->methods->cancel(io_handle);
}

IO_INLINE(io_Err)
io_Handle_submit(io_Handle* io_handle, io_Op* iot)
{
    return io_handle->methods->submit(io_handle, iot);
}

IO_INLINE(int)
io_Handle_get_fd(const io_Handle* io_handle)
{
    return io_handle->methods->get_fd(io_handle);
}

IO_INLINE(void)
io_Handle_set_timeout(io_Handle* io_handle, io_OpType type, io_Duration duration)
{
    io_handle->methods->set_timeout(io_handle, type, duration);
}

IO_INLINE(void)
io_Handle_destroy(io_Handle* io_handle)
{
    io_handle->methods->destroy(io_handle);
}

typedef struct io_Reactor {
    io_Err (*run)(void* self, io_Duration timeout);
    io_Handle* (*create_handle)(void* self, int fd);
    void (*interrupt)(void* self);
    void (*destroy)(void* self);
} io_Reactor;

IO_INLINE(io_Err)
io_Reactor_run(io_Reactor* io_service, int timeout_ms)
{
    return io_service->run(io_service, timeout_ms);
}

IO_INLINE(io_Handle*)
io_Reactor_create_handle(io_Reactor* io_service, int fd)
{
    return io_service->create_handle(io_service, fd);
}

IO_INLINE(void)
io_Reactor_interrupt(io_Reactor* io_service)
{
    io_service->interrupt(io_service);
}

IO_INLINE(void)
io_Reactor_destroy(io_Reactor* io_service)
{
    if (io_service->destroy)
        io_service->destroy(io_service);
}

#endif
