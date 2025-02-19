/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_POLL_H
#define IO_POLL_H

#include <io/config.h>

#ifdef IO_WITH_POLL
#include <io/err.h>
#include <io/hashmap.h>
#include <io/loop.h>
#include <io/obj_pool.h>
#include <io/other_err.h>
#include <io/queue.h>
#include <io/reactor.h>
#include <io/system_call.h>
#include <io/task.h>
#include <io/timer.h>
#include <io/vec.h>

#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>

/* forward declarations begin */

typedef struct io_Poll io_Poll;

/* forward declarations end */
/* io_poll_handle begin */

typedef struct io_PollHandle {
    io_Handle base;
    struct io_PollHandle* next;
    struct io_PollHandle* prev;
    io_Poll* poll;
    io_Op* ops[IO_OP_MAX];
    io_Duration timeout[IO_OP_MAX];
    io_Timer timer[IO_OP_MAX];
    io_Mutex mtx;
    int fd;
} io_PollHandle;

IO_INLINE(uint32_t)
io_hash_fd(int fd)
{
    return (uint32_t)fd;
}

IO_INLINE(bool)
io_cmp_int(int a, int b)
{
    return a == b;
}

IO_INLINE(void)
io_PollHandleVec_deinit_entry(io_PollHandle** handle)
{
    io_Handle_destroy(&(*handle)->base);
}

IO_DEFINE_HASHMAP(io_FdToIdxMap, int, size_t, io_hash_fd, io_cmp_int, ((int)-1))
IO_DEFINE_VEC(io_PollFdVec, struct pollfd, (void))
IO_DEFINE_VEC(io_PollHandleVec, io_PollHandle*, io_PollHandleVec_deinit_entry)
IO_DEFINE_OBJ_POOL(io_PollHandlePool, io_PollHandle, prev, next)

/* io_poll_handle end */
/* io_poll_handle_map begin */

typedef struct io_PollHandleMap {
    io_FdToIdxMap map;
    io_PollHandleVec handles;
    io_Mutex mtx;
} io_PollHandleMap;

IO_INLINE(void)
io_PollHandleMap_init(io_PollHandleMap* map, io_Allocator* allocator)
{
    io_FdToIdxMap_init(&map->map, allocator);
    io_PollHandleVec_init(&map->handles, allocator);
    io_Mutex_init(&map->mtx);
}

IO_INLINE(void)
io_PollHandleMap_deinit(io_PollHandleMap* map)
{
    io_FdToIdxMap_deinit(&map->map);
    io_PollHandleVec_deinit(&map->handles);
    io_Mutex_deinit(&map->mtx);
}

/** th_poll_handle_map_set
 * @brief Sets the poll handle for the given file descriptor.
 */
IO_INLINE(io_Err)
io_PollHandleMap_set(io_PollHandleMap* map, int fd, io_PollHandle* handle)
{
    io_Mutex_lock(&map->mtx);
    io_Err err = IO_ERR_OK;
    size_t idx = 0;
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    if (iter == NULL) {
        if ((err = io_PollHandleVec_push_back(&map->handles, handle))) {
            goto cleanup;
        }
        idx = (io_PollHandleVec_size(&map->handles) - 1);
        if ((err = io_FdToIdxMap_set(&map->map, fd, idx))) {
            io_PollHandleVec_pop_back(&map->handles);
            goto cleanup;
        }
    } else {
        idx = iter->value;
    }
    *io_PollHandleVec_at(&map->handles, idx) = handle;
cleanup:
    io_Mutex_unlock(&map->mtx);
    return err;
}

/* io_PollHandleMap_try_get
 * @brief Get the poll handle for the given file descriptor.
 * @param map The handle map.
 * @param fd The file descriptor.
 * @return The poll handle, NULL if the handle wasn't found.
 */
IO_INLINE(io_PollHandle*)
io_PollHandleMap_try_get(io_PollHandleMap* map, int fd)
{
    io_Mutex_lock(&map->mtx);
    io_PollHandle* handle = NULL;
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    if (iter) {
        handle = *io_PollHandleVec_at(&map->handles, iter->value);
    }
    io_Mutex_unlock(&map->mtx);
    return handle;
}

IO_INLINE(void)
io_PollHandleMap_remove(io_PollHandleMap* map, int fd)
{
    io_Mutex_lock(&map->mtx);
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    IO_ASSERT(iter, "Must not remove a non-existent handle");
    if (iter) {
        size_t idx = iter->value;
        io_FdToIdxMap_erase(&map->map, iter);
        if (idx != (io_PollHandleVec_size(&map->handles) - 1)) {
            io_FdToIdxMap_iter last = io_FdToIdxMap_find(&map->map, io_PollHandleVec_back(&map->handles)->fd);
            last->value = idx;
            *(io_PollHandleVec_at(&map->handles, idx)) = (io_PollHandleVec_back(&map->handles));
        }
        io_PollHandleVec_resize(&map->handles, (io_PollHandleVec_size(&map->handles) - 1));
    }
    io_Mutex_unlock(&map->mtx);
}

/* io_poll_handle_map end */
/* io_PollTimer begin */

typedef struct io_PollTimer {
    io_Mutex mtx;
    io_Timer timer;
    bool armed;
} io_PollTimer;

IO_INLINE(void)
io_PollTimer_init(io_PollTimer* timer)
{
    io_Mutex_init(&timer->mtx);
    io_Timer_init(&timer->timer, (io_Duration)0);
    timer->armed = false;
}

IO_INLINE(void)
io_PollTimer_update(io_PollTimer* timer, io_Duration duration)
{
    io_Mutex_lock(&timer->mtx);
    if (timer->armed && io_Timer_remaining(&timer->timer) > duration) {
        io_Timer_set(&timer->timer, duration);
    } else if (!timer->armed) {
        io_Timer_set(&timer->timer, duration);
        timer->armed = true;
    }
    io_Mutex_unlock(&timer->mtx);
}

IO_INLINE(io_Duration)
io_PollTimer_retrieve(io_PollTimer* timer)
{
    io_Duration duration = IO_TIMEOUT_INFINITE;
    io_Mutex_lock(&timer->mtx);
    if (timer->armed) {
        duration = io_Timer_remaining(&timer->timer);
        timer->armed = false;
    }
    io_Mutex_unlock(&timer->mtx);
    return duration;
}

IO_INLINE(void)
io_PollTimer_deinit(io_PollTimer* timer)
{
    io_Mutex_deinit(&timer->mtx);
}

/* io_PollTimer end */
/* io_PollFds begin */

typedef struct io_PollFds {
    io_PollFdVec fds;
    io_PollFdVec pending_fds;
    io_Mutex mtx;
} io_PollFds;

IO_INLINE(void)
io_PollFds_init(io_PollFds* fds, io_Allocator* allocator)
{
    io_PollFdVec_init(&fds->fds, allocator);
    io_PollFdVec_init(&fds->pending_fds, allocator);
    io_Mutex_init(&fds->mtx);
}

IO_INLINE(void)
io_PollFds_deinit(io_PollFds* fds)
{
    io_PollFdVec_deinit(&fds->fds);
    io_PollFdVec_deinit(&fds->pending_fds);
    io_Mutex_deinit(&fds->mtx);
}

IO_INLINE(io_Err)
io_PollFds_add_fd(io_PollFds* fds, struct pollfd pfd)
{
    io_Mutex_lock(&fds->mtx);
    io_Err err = io_PollFdVec_push_back(&fds->fds, pfd);
    io_Mutex_unlock(&fds->mtx);
    return err;
}

IO_INLINE(io_Err)
io_PollFds_update(io_PollFds* fds)
{
    io_Err err = IO_ERR_OK;
    io_Mutex_lock(&fds->mtx);
    size_t pending = io_PollFdVec_size(&fds->pending_fds);
    for (size_t i = pending; i--;) {
        if ((err = io_PollFdVec_push_back(&fds->fds, *io_PollFdVec_at(&fds->pending_fds, i)))) {
            break;
        }
    }
    io_PollFdVec_clear(&fds->pending_fds);
    io_Mutex_unlock(&fds->mtx);
    return err;
}

IO_INLINE(io_PollFdVec*)
io_PollFds_get(io_PollFds* fds)
{
    return &fds->fds;
}

/* io_PollFds end */
/* io_poll begin */

struct io_Poll {
    io_Reactor base;
    io_Allocator* allocator;
    io_Loop* loop;
    io_PollHandlePool handle_allocator;
    io_PollHandleMap handles;
    io_PollFds fds;
    io_PollTimer timer;
    int interrupt_fds[2];
};

/* th_poll_handle implementation begin */

IO_INLINE(io_Err)
io_PollHandle_submit(void* self, io_Op* op)
{
    io_PollHandle* handle = (io_PollHandle*)self;
    io_Poll* poll = handle->poll;
    io_Op_set_flags(op, IO_OP_TRYIO);
    io_Op_perform(op);
    if (io_Op_flags(op) & IO_OP_COMPLETED) {
        return IO_ERR_OK;
    }
    io_Op_clear_flags(op, IO_OP_TRYIO);
    io_OpType op_type = op->type;
    io_Mutex_lock(&handle->mtx);
    handle->ops[op_type] = op;
    struct pollfd pfd = {.fd = handle->fd, .events = 0};
    switch (op_type) {
    case IO_OP_READ:
        pfd.events = POLLIN;
        break;
    case IO_OP_WRITE:
        pfd.events = POLLOUT;
        break;
    default:
        IO_ASSERT(0, "Invalid operation type");
        break;
    }
    if (handle->timeout[op_type] != IO_TIMEOUT_INFINITE) {
        io_Timer_set(&handle->timer[op_type], handle->timeout[op_type]);
        io_PollTimer_update(&poll->timer, handle->timeout[op_type]);
    }
    io_Mutex_unlock(&handle->mtx);
    io_Err err = io_PollFds_add_fd(&poll->fds, pfd);
    if (err) {
        return err;
    }
    io_Loop_increase_task_count(poll->loop);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_PollHandle_set_timeout(void* self, io_OpType op_type, io_Duration duration)
{
    io_PollHandle* handle = self;
    io_Mutex_lock(&handle->mtx);
    handle->timeout[op_type] = duration;
    io_Mutex_unlock(&handle->mtx);
}

IO_INLINE(void)
io_PollHandle_cancel(void* self)
{
    io_PollHandle* handle = self;
    io_Mutex_lock(&handle->mtx);
    for (int i = IO_OP_MAX; --i;) {
        io_Op* op = IO_MOVE_PTR(handle->ops[i - 1]);
        if (op) {
            io_Op_abort(op, io_SystemErr(IO_ECANCELED));
            io_Loop_decrease_task_count(handle->poll->loop);
        }
    }
    io_Mutex_unlock(&handle->mtx);
}

IO_INLINE(int)
io_PollHandle_get_fd(const void* self)
{
    const io_PollHandle* handle = self;
    return handle->fd;
}

IO_INLINE(void)
io_PollHandle_destroy(void* self)
{
    io_PollHandle* handle = self;
    io_PollHandleMap_remove(&handle->poll->handles, handle->fd);
    close(handle->fd);
    io_Allocator_free(&handle->poll->handle_allocator.base, self);
}

IO_INLINE(void)
io_PollHandle_lock(io_PollHandle* handle)
{
    io_Mutex_lock(&handle->mtx);
}

IO_INLINE(void)
io_PollHandle_unlock(io_PollHandle* handle)
{
    io_Mutex_unlock(&handle->mtx);
}

IO_INLINE(void)
io_PollHandle_init(io_PollHandle* handle, io_Poll* poll, int fd)
{
    static io_HandleMethods methods = {
        .submit = io_PollHandle_submit,
        .cancel = io_PollHandle_cancel,
        .destroy = io_PollHandle_destroy,
        .set_timeout = io_PollHandle_set_timeout,
        .get_fd = io_PollHandle_get_fd,
    };
    handle->base.methods = &methods;
    for (size_t idx = IO_OP_MAX - 1; idx--;) {
        handle->ops[idx] = NULL;
    }
    handle->poll = poll;
    handle->fd = fd;
    handle->timeout[0] = IO_TIMEOUT_INFINITE;
    handle->timeout[1] = IO_TIMEOUT_INFINITE;
    io_Timer_init(&handle->timer[0], IO_TIMEOUT_INFINITE);
    io_Timer_init(&handle->timer[1], IO_TIMEOUT_INFINITE);
    io_Mutex_init(&handle->mtx);
}

/* th_poll_handle implementation end */

IO_INLINE(io_Handle*)
io_Poll_create_handle(void* self, int fd)
{
    io_Poll* poll = self;
    io_PollHandle* handle = io_Allocator_alloc(&poll->handle_allocator.base, sizeof(io_PollHandle));
    if (!handle)
        return NULL;
    io_PollHandle_init(handle, poll, fd);
    io_PollHandleMap_set(&poll->handles, handle->fd, handle);
    return &handle->base;
}

IO_INLINE(io_Err)
io_Poll_run(void* self, io_Duration timeout)
{
    io_Poll* service = self;
    if (timeout == IO_TIMEOUT_INFINITE) {
        io_Duration earliest = io_PollTimer_retrieve(&service->timer);
        if (earliest != IO_TIMEOUT_INFINITE) {
            timeout = earliest;
        }
    }
    int timeout_ms = io_Duration_to_ms(timeout);
    io_Err err = io_PollFds_update(&service->fds);
    if (err) {
        return err;
    }
    io_PollFdVec* fds = io_PollFds_get(&service->fds);
    nfds_t nfds = (nfds_t)io_PollFdVec_size(fds);
    int ret = io_poll(io_PollFdVec_begin(fds), nfds, timeout_ms);
    if (ret <= 0) {
        return IO_ERR_OK;
    }

    if (io_PollFdVec_begin(fds)->revents & POLLIN) {
        IO_ASSERT(io_PollFdVec_begin(fds)->fd == service->interrupt_fds[0], "Invalid interrupt fd");
        char c[512];
        (void)read(service->interrupt_fds[0], &c, sizeof(c));
    }

    size_t reenqueue = 1;
    for (size_t i = 1; i < nfds; ++i) {
        struct pollfd* pfd = io_PollFdVec_at(fds, i);
        short revents = pfd->revents;
        short events = pfd->events & (POLLIN | POLLOUT);
        int op_index = 0;
        switch (events) {
        case POLLIN:
            op_index = IO_OP_READ;
            break;
        case POLLOUT:
            op_index = IO_OP_WRITE;
            break;
        default:
            continue;
            break;
        }
        io_PollHandle* handle = io_PollHandleMap_try_get(&service->handles, pfd->fd);
        if (!handle) // handle was removed
            continue;
        io_PollHandle_lock(handle);
        io_Op* op = handle->ops[op_index];
        if (revents && op) {
            if (revents & events) {
                io_Loop_push_task(service->loop, &op->base);
            } else if (revents & POLLHUP) {
                io_Op_abort(op, IO_ERR_EOF);
            } else if (revents & (POLLERR | POLLPRI)) {
                io_Op_abort(op, io_SystemErr(IO_EIO));
            } else if (revents & POLLNVAL) {
                io_Op_abort(op, io_SystemErr(IO_EBADF));
            } else {
                io_Op_abort(op, IO_ERR_UNKNOWN);
            }
            io_Loop_decrease_task_count(service->loop);
            handle->ops[op_index] = NULL;
        } else if (op) { // reenqueue
            if (handle->timeout[op_index] != IO_TIMEOUT_INFINITE
                && io_Timer_expired(&handle->timer[op_index])) {
                io_Op_abort(op, io_SystemErr(IO_ETIMEDOUT));
                io_Loop_decrease_task_count(service->loop);
                handle->ops[op_index] = NULL;
            } else {
                if (handle->timeout[op_index] != IO_TIMEOUT_INFINITE) {
                    io_PollTimer_update(&service->timer, handle->timeout[op_index]);
                }
                if (reenqueue < i) {
                    *io_PollFdVec_at(fds, reenqueue) = *pfd;
                }
                ++reenqueue;
            }
        }
        io_PollHandle_unlock(handle);
    }
    io_PollFdVec_resize(fds, reenqueue);
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Poll_interrupt(void* self)
{
    io_Poll* service = self;
    char c = 0;
    (void)io_write(service->interrupt_fds[1], &c, 1);
}

IO_INLINE(void)
io_Poll_destroy(void* self)
{
    io_Poll* service = self;
    io_PollHandleMap_deinit(&service->handles);
    io_PollHandlePool_deinit(&service->handle_allocator);
    io_PollFds_deinit(&service->fds);
    io_PollTimer_deinit(&service->timer);
    io_close(service->interrupt_fds[0]);
    io_close(service->interrupt_fds[1]);
    io_free(service->allocator, self);
}

IO_INLINE(io_Err)
io_Poll_create(io_Reactor** out, io_Loop* loop, io_Allocator* allocator)
{
    io_Poll* service = io_alloc(allocator, sizeof(io_Poll));
    if (!service) {
        return io_SystemErr(IO_ENOMEM);
    }
    service->base.run = io_Poll_run;
    service->base.destroy = io_Poll_destroy;
    service->base.create_handle = io_Poll_create_handle;
    service->base.interrupt = io_Poll_interrupt;
    service->allocator = allocator;
    service->loop = loop;
    io_Err err = IO_ERR_OK;
    if (io_pipe(service->interrupt_fds) == -1) {
        err = io_SystemErr(errno);
        goto on_pipe_err;
    }
    io_PollFds_init(&service->fds, allocator);
    struct pollfd pfd = {.fd = service->interrupt_fds[0], .events = POLLIN};
    if ((err = io_PollFds_add_fd(&service->fds, pfd))) {
        goto on_PollFds_err;
    }
    io_PollHandleMap_init(&service->handles, allocator);
    io_PollHandlePool_init(&service->handle_allocator, allocator, 16, 8 * 1024);
    io_PollTimer_init(&service->timer);
    *out = &service->base;
    return IO_ERR_OK;
on_PollFds_err:
    io_close(service->interrupt_fds[0]);
    io_close(service->interrupt_fds[1]);
on_pipe_err:
    io_free(allocator, service);
    return err;
}

#endif
#endif
