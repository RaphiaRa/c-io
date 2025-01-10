#ifndef IO_POLL_H
#define IO_POLL_H

#include <io/io_config.h>

#ifdef IO_WITH_POLL
#include <io/io_err.h>
#include <io/io_hashmap.h>
#include <io/io_loop.h>
#include <io/io_obj_pool.h>
#include <io/io_queue.h>
#include <io/io_reactor.h>
#include <io/io_task.h>
#include <io/io_timer.h>
#include <io/io_vec.h>

#include <poll.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

/* forward declarations begin */

typedef struct io_Poll io_Poll;

/* forward declarations end */
/* io_poll_handle begin */

typedef struct io_PollHandle {
    io_Handle base;
    io_Timer timer;
    io_PollHandle* next;
    io_PollHandle* prev;
    io_Poll* poll;
    io_Op* iot[IO_OP_MAX];
    int fd;
    bool timeout_enabled;
} io_PollHandle;

IO_INLINE(void)
io_PollHandle_destroy(io_PollHandle** handle)
{
    io_Handle_destroy(&(*handle)->base);
}

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

IO_DEFINE_HASHMAP(io_FdToIdxMap, int, size_t, io_hash_fd, io_cmp_int, ((int)-1))
IO_DEFINE_VEC(io_PollFdVec, struct pollfd, (void))
IO_DEFINE_VEC(io_PollHandleVec, io_PollHandle*, io_PollHandle_destroy)
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
    th_fd_to_idx_map_deinit(&map->map);
    io_PollHandleVec_deinit(&map->handles);
}

/** th_poll_handle_map_set
 * @brief Sets the poll handle for the given file descriptor.
 */
IO_INLINE(void)
io_PollHandleMap_map_set(io_PollHandleMap* map, int fd, io_PollHandle* handle)
{
    size_t idx = 0;
    io_FdToIdxMap_iter iter = io_FdToIdxMap_find(&map->map, fd);
    if (iter == NULL) {
        io_PollHandleVec_push_back(&map->handles, handle);
        idx = io_PollHandleVec_size(&map->handles);
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
            io_FdToIdxMap_iter last = io_FdToIdxMap_find(&map->map, (*io_PollHandleVec_end(&map->handles))->fd);
            last->value = idx;
            *(io_PollHandleVec_at(&map->handles, idx)) = *(io_PollHandleVec_end(&map->handles));
        }
        io_PollHandleVec_resize(&map->handles, (io_PollHandleVec_size(&map->handles) - 1));
    }
}

/* io_poll_handle_map end */

struct io_Poll {
    io_Reactor base;
    io_Allocator* allocator;
    io_Loop* loop;
    io_PollHandlePool handle_allocator;
    io_PollHandleMap handles;
    io_PollFdVec fds;
};

IO_INLINE(io_Err)
io_Poll_create(io_Reactor** out, io_Loop* loop, io_Allocator* allocator);

#endif /* IO_HAVE_POLL */
#endif
