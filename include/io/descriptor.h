/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_DESCRIPTOR_H
#define IO_DESCRIPTOR_H

#include <io/config.h>
#include <io/context.h>
#include <io/reactor.h>

typedef struct io_Descriptor {
    io_Context* context;
    io_Handle* handle;
} io_Descriptor;

IO_INLINE(void)
io_Descriptor_init(io_Descriptor* descriptor, io_Context* context)
{
    descriptor->context = context;
    descriptor->handle = NULL;
}

IO_INLINE(void)
io_Descriptor_clear_fd(io_Descriptor* descriptor)
{
    if (descriptor->handle) {
        io_Handle_destroy(descriptor->handle);
        descriptor->handle = NULL;
    }
}

IO_INLINE(int)
io_Descriptor_get_fd(const io_Descriptor* descriptor)
{
    if (descriptor->handle == NULL)
        return -1;
    return io_Handle_get_fd(descriptor->handle);
}

IO_INLINE(void)
io_Descriptor_set_fd(io_Descriptor* descriptor, int fd)
{
    if (descriptor->handle) {
        io_Descriptor_clear_fd(descriptor);
    }
    descriptor->handle = io_Reactor_create_handle(io_Context_next_loop(descriptor->context)->reactor, fd);
}

IO_INLINE(io_Context*)
io_Descriptor_get_context(io_Descriptor* descriptor)
{
    return descriptor->context;
}

IO_INLINE(void)
io_Descriptor_cancel(io_Descriptor* descriptor)
{
    if (descriptor->handle) {
        io_Handle_cancel(descriptor->handle);
    }
}

IO_INLINE(io_Err)
io_Descriptor_set_non_blocking(io_Descriptor* descriptor, bool non_blocking)
{
    int fd = io_Descriptor_get_fd(descriptor);
    int flags = io_fcntl(fd, F_GETFL);
    if (flags == -1) {
        return io_SystemErr(errno);
    }
    if (non_blocking) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    if (io_fcntl(fd, F_SETFL, flags) == -1) {
        return io_SystemErr(errno);
    }
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_Descriptor_get_non_blocking(const io_Descriptor* descriptor, bool* non_blocking)
{
    int fd = io_Descriptor_get_fd(descriptor);
    int flags = io_fcntl(fd, F_GETFL);
    if (flags == -1) {
        return io_SystemErr(errno);
    }
    *non_blocking = (flags & O_NONBLOCK) != 0;
    return IO_ERR_OK;
}

IO_INLINE(io_Err)
io_Descriptor_set_timeout(io_Descriptor* descriptor, io_OpType type, io_Duration duration)
{
    if (descriptor->handle == NULL) {
        return io_SystemErr(EBADF);
    }
    io_Handle_set_timeout(descriptor->handle, type, duration);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Descriptor_close(io_Descriptor* descriptor)
{
    io_Descriptor_clear_fd(descriptor);
}

#define DEFINE_DESCRIPTOR_WRAPPERS(P, B)                                 \
    IO_INLINE(void)                                                      \
    P##_close(P* descriptor)                                             \
    {                                                                    \
        B##_close(&descriptor->base);                                    \
    }                                                                    \
                                                                         \
    IO_INLINE(void)                                                      \
    P##_set_fd(P* descriptor, int fd)                                    \
    {                                                                    \
        B##_set_fd(&descriptor->base, fd);                               \
    }                                                                    \
                                                                         \
    IO_INLINE(int)                                                       \
    P##_get_fd(const P* descriptor)                                      \
    {                                                                    \
        return B##_get_fd(&descriptor->base);                            \
    }                                                                    \
                                                                         \
    IO_INLINE(io_Context*)                                               \
    P##_get_context(P* descriptor)                                       \
    {                                                                    \
        return B##_get_context(&descriptor->base);                       \
    }                                                                    \
                                                                         \
    IO_INLINE(io_Err)                                                    \
    P##_set_non_blocking(P* descriptor, bool non_blocking)               \
    {                                                                    \
        return B##_set_non_blocking(&descriptor->base, non_blocking);    \
    }                                                                    \
                                                                         \
    IO_INLINE(io_Err)                                                    \
    P##_get_non_blocking(const P* descriptor, bool* non_blocking)        \
    {                                                                    \
        return B##_get_non_blocking(&descriptor->base, non_blocking);    \
    }                                                                    \
                                                                         \
    IO_INLINE(io_Err)                                                    \
    P##_set_timeout(P* descriptor, io_OpType type, io_Duration duration) \
    {                                                                    \
        return B##_set_timeout(&descriptor->base, type, duration);       \
    }                                                                    \
                                                                         \
    IO_INLINE(void)                                                      \
    P##_cancel(P* descriptor)                                            \
    {                                                                    \
        B##_cancel(&descriptor->base);                                   \
    }

#endif
