#ifndef IO_REACTOR_H
#define IO_REACTOR_H

#include <stdbool.h>

#include <io/io_config.h>
#include <io/io_err.h>
#include <io/io_task.h>

typedef struct io_HandleMethods {
    void (*cancel)(void* self);
    void (*submit)(void* self, io_Op* op);
    void (*enable_timeout)(void* self, bool enabled);
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

IO_INLINE(void)
io_Handle_submit(io_Handle* io_handle, io_Op* iot)
{
    io_handle->methods->submit(io_handle, iot);
}

IO_INLINE(int)
io_Handle_get_fd(const io_Handle* io_handle)
{
    return io_handle->methods->get_fd(io_handle);
}

IO_INLINE(void)
io_Handle_enable_timeout(io_Handle* io_handle, bool enabled)
{
    io_handle->methods->enable_timeout(io_handle, enabled);
}

IO_INLINE(void)
io_Handle_destroy(io_Handle* io_handle)
{
    io_handle->methods->destroy(io_handle);
}

typedef struct io_Reactor {
    void (*run)(void* self, int timeout_ms);
    io_Handle* (*create_handle)(void* self, int fd);
    void (*destroy)(void* self);
} io_Reactor;

IO_INLINE(void)
io_Reactor_run(io_Reactor* io_service, int timeout_ms)
{
    io_service->run(io_service, timeout_ms);
}

IO_INLINE(io_Handle*)
io_Reactor_create_handle(io_Reactor* io_service, int fd)
{
    return io_service->create_handle(io_service, fd);
}

IO_INLINE(void)
io_Reactor_destroy(io_Reactor* io_service)
{
    if (io_service->destroy)
        io_service->destroy(io_service);
}

#endif
