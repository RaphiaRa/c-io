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
#include <io/task.h>
#include <io/timer.h>
#include <io/vec.h>

#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/types.h>

/* forward declarations begin */

typedef struct io_Poll io_Poll;

/* forward declarations end */
/* io_poll_handle begin */

typedef struct io_PollHandle {
    io_Handle base;
    io_Timer timer;
    struct io_PollHandle* next;
    struct io_PollHandle* prev;
    io_Poll* poll;
    io_Op* ops[IO_OP_MAX];
    int fd;
    bool timeout_enabled;
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
} io_PollHandleMap;

IO_INLINE(void)
io_PollHandleMap_init(io_PollHandleMap* map, io_Allocator* allocator)
{
    io_FdToIdxMap_init(&map->map, allocator);
    io_PollHandleVec_init(&map->handles, allocator);
}

IO_INLINE(void)
io_PollHandleMap_deinit(io_PollHandleMap* map)
{
    io_FdToIdxMap_deinit(&map->map);
    io_PollHandleVec_deinit(&map->handles);
}

/** th_poll_handle_map_set
 * @brief Sets the poll handle for the given file descriptor.
 */
IO_INLINE(void)
io_PollHandleMap_set(io_PollHandleMap* map, int fd, io_PollHandle* handle)
{
    size_t idx = 0;
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    if (iter == NULL) {
        io_PollHandleVec_push_back(&map->handles, handle);
        idx = (io_PollHandleVec_size(&map->handles) - 1);
        io_FdToIdxMap_set(&map->map, fd, idx);
    } else {
        idx = iter->value;
    }
    *io_PollHandleVec_at(&map->handles, idx) = handle;
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
    io_PollHandle* handle = NULL;
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    if (iter) {
        handle = *io_PollHandleVec_at(&map->handles, iter->value);
    }
    return handle;
}

IO_INLINE(void)
io_PollHandleMap_remove(io_PollHandleMap* map, int fd)
{
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
}

/* io_poll_handle_map end */

struct io_Poll {
    io_Reactor base;
    io_Loop* loop;
    io_PollHandlePool handle_allocator;
    io_PollHandleMap handles;
    io_PollFdVec fds;
};

/* th_poll_handle implementation begin */

IO_INLINE(void)
io_PollHandle_submit(void* self, io_Op* op)
{
    io_PollHandle* handle = (io_PollHandle*)self;
    io_Poll* poll = handle->poll;
    io_Op_set_flags(op, IO_OP_TRYIO);
    io_Op_perform(op);
    if (io_Op_flags(op) & IO_OP_COMPLETED) {
        return;
    }
    io_Op_clear_flags(op, IO_OP_TRYIO);
    io_OpType op_type = op->type;
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
    if (handle->timeout_enabled) {
        io_Err err = io_Timer_set(&handle->timer, io_Seconds(IO_DEFAULT_TIMEOUT));
        if (!io_ok(err)) {
            handle->timeout_enabled = false;
        }
    }
    io_PollFdVec_push_back(&poll->fds, pfd);
    io_Loop_increase_task_count(poll->loop);
}

IO_INLINE(void)
io_PollHandle_cancel(void* self)
{
    io_PollHandle* handle = self;
    for (int i = (IO_OP_MAX - 1); --i;) {
        io_Op* op = handle->ops[i];
        if (op) {
            handle->ops[i] = NULL;
            io_Op_abort(op, io_SystemErr_make(IO_ECANCELED));
            io_Op_destroy(op);
            io_Loop_decrease_task_count(handle->poll->loop);
        }
    }
}

IO_INLINE(int)
io_PollHandle_get_fd(const void* self)
{
    const io_PollHandle* handle = self;
    return handle->fd;
}

IO_INLINE(void)
io_PollHandle_enable_timeout(void* self, bool enable)
{
    io_PollHandle* handle = self;
    handle->timeout_enabled = enable;
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
io_PollHandle_init(io_PollHandle* handle, io_Poll* poll, int fd)
{
    static io_HandleMethods methods = {
        .submit = io_PollHandle_submit,
        .cancel = io_PollHandle_cancel,
        .destroy = io_PollHandle_destroy,
        .enable_timeout = io_PollHandle_enable_timeout,
        .get_fd = io_PollHandle_get_fd,
    };
    handle->base.methods = &methods;
    io_Timer_init(&handle->timer);
    for (size_t idx = IO_OP_MAX - 1; --idx;) {
        handle->ops[idx] = NULL;
    }
    handle->poll = poll;
    handle->fd = fd;
    handle->timeout_enabled = false;
}

/* th_poll_handle implementation end */

IO_INLINE(io_Handle*)
io_Poll_create_handle(void* self, int fd)
{
    io_Poll* poll = self;
    io_PollHandle* handle = io_Allocator_alloc(&poll->handle_allocator.base, sizeof(io_PollHandle));
    IO_REQUIRE(handle, "Out of memory");
    io_PollHandle_init(handle, poll, fd);
    io_PollHandleMap_set(&poll->handles, handle->fd, handle);
    return &handle->base;
}

IO_INLINE(void)
io_Poll_run(void* self, int timeout_ms)
{
    io_Poll* service = self;
    nfds_t nfds = io_PollFdVec_size(&service->fds);
    int ret = poll(io_PollFdVec_begin(&service->fds), nfds, timeout_ms);
    if (ret <= 0) {
        return;
    }

    size_t reenqueue = 0;
    for (size_t i = 0; i < nfds; ++i) {
        io_PollHandle* handle = io_PollHandleMap_try_get(&service->handles, io_PollFdVec_at(&service->fds, i)->fd);
        if (!handle) // handle was removed
            continue;
        short revents = io_PollFdVec_at(&service->fds, i)->revents;
        short events = io_PollFdVec_at(&service->fds, i)->events & (POLLIN | POLLOUT);
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
        io_Op* op = handle->ops[op_index];
        if (revents && op) {
            if (revents & events) {
                io_Loop_push_task(service->loop, &op->base);
                io_Loop_decrease_task_count(service->loop);
            } else if (revents & POLLHUP) {
                io_Op_abort(op, IO_ERR_EOF);
                io_Loop_decrease_task_count(service->loop);
            } else if (revents & (POLLERR | POLLPRI)) {
                io_Op_abort(op, io_SystemErr_make(IO_EIO));
                io_Loop_decrease_task_count(service->loop);
            } else if (revents & POLLNVAL) {
                io_Op_abort(op, io_SystemErr_make(IO_EBADF));
                io_Loop_decrease_task_count(service->loop);
            } else {
                io_Op_abort(op, IO_ERR_UNKNOWN);
                io_Loop_decrease_task_count(service->loop);
            }
            handle->ops[op_index] = NULL;
        } else if (op) { // reenqueue
            if (handle->timeout_enabled && io_Timer_expired(&handle->timer)) {
                io_Op_abort(op, io_SystemErr_make(IO_ETIMEDOUT));
                io_Loop_decrease_task_count(service->loop);
                handle->ops[op_index] = NULL;
            } else {
                if (reenqueue < i) {
                    *io_PollFdVec_at(&service->fds, reenqueue) = *io_PollFdVec_at(&service->fds, i);
                }
                ++reenqueue;
            }
        }
        // handles without op were cancelled, so we don't need to reenqueue them
    }
    io_PollFdVec_resize(&service->fds, reenqueue);
}

IO_INLINE(void)
io_Poll_destroy(void* self)
{
    io_Poll* service = self;
    io_PollFdVec_deinit(&service->fds);
    io_PollHandleMap_deinit(&service->handles);
    io_PollHandlePool_deinit(&service->handle_allocator);
    io_free(self);
}

IO_INLINE(io_Reactor*)
io_Poll_create(io_Loop* loop)
{
    io_Poll* service = io_alloc(sizeof(io_Poll));
    IO_REQUIRE(service, "Out of memory");
    service->base.run = io_Poll_run;
    service->base.destroy = io_Poll_destroy;
    service->base.create_handle = io_Poll_create_handle;
    service->loop = loop;
    io_PollFdVec_init(&service->fds, io_DefaultAllocator());
    io_PollHandleMap_init(&service->handles, io_DefaultAllocator());
    io_PollHandlePool_init(&service->handle_allocator, io_DefaultAllocator(), 16, 8 * 1024);
    return &service->base;
}

#endif
#endif
