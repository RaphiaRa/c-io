/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_OBJ_POOL_H
#define IO_OBJ_POOL_H

#include <io/allocator.h>
#include <io/assert.h>
#include <io/list.h>

/** Generic object pool allocator.
 * The pool allocator is a allocator that allocates objects from a pool of fixed-size blocks.
 * It can be used with any object that has a next and prev pointer.
 */
#define IO_DEFINE_OBJ_POOL(NAME, T, PREV, NEXT)                                                   \
    IO_DEFINE_LIST(NAME##_list, T, PREV, NEXT)                                                    \
    typedef struct NAME {                                                                         \
        io_Allocator base;                                                                        \
        NAME##_list free_list;                                                                    \
        NAME##_list used_list;                                                                    \
        io_Allocator* allocator;                                                                  \
        size_t count;                                                                             \
        size_t max;                                                                               \
    } NAME;                                                                                       \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, io_Allocator* allocator, size_t initial, size_t max) IO_MAYBE_UNUSED; \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool) IO_MAYBE_UNUSED;                                                    \
                                                                                                  \
    IO_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t) IO_MAYBE_UNUSED;                                             \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr) IO_MAYBE_UNUSED;                                           \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_init(NAME* pool, io_Allocator* allocator, size_t initial, size_t max)                  \
    {                                                                                             \
        IO_ASSERT(allocator != NULL, "Invalid allocator");                                        \
        IO_ASSERT(max > 0, "Invalid max");                                                        \
        static io_AllocatorMethods methods = {                                                    \
            .alloc = NAME##_alloc,                                                                \
            .realloc = NULL,                                                                      \
            .free = NAME##_free,                                                                  \
        };                                                                                        \
        pool->base.methods = &methods;                                                            \
        pool->allocator = allocator;                                                              \
        pool->count = 0;                                                                          \
        pool->max = max;                                                                          \
        pool->used_list = (NAME##_list){0};                                                       \
        pool->free_list = (NAME##_list){0};                                                       \
        for (size_t i = 0; i < initial; i++) {                                                    \
            T* item = (T*)io_Allocator_alloc(pool->allocator, sizeof(T));                         \
            if (item) {                                                                           \
                NAME##_list_push_back(&pool->free_list, item);                                    \
                ++pool->count;                                                                    \
            }                                                                                     \
        }                                                                                         \
    }                                                                                             \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_deinit(NAME* pool)                                                                     \
    {                                                                                             \
        T* item = NULL;                                                                           \
        while ((item = NAME##_list_pop_front(&pool->free_list))) {                                \
            io_Allocator_free(pool->allocator, item);                                             \
        }                                                                                         \
        item = NAME##_list_pop_front(&pool->used_list);                                           \
        IO_ASSERT(item == NULL, "Memory leak detected");                                          \
    }                                                                                             \
                                                                                                  \
    IO_INLINE(void*)                                                                              \
    NAME##_alloc(void* self, size_t size)                                                         \
    {                                                                                             \
        IO_ASSERT(size == sizeof(T), "Invalid size");                                             \
        (void)size;                                                                               \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = NAME##_list_pop_front(&pool->free_list);                                        \
        if (item == NULL) {                                                                       \
            if (pool->count < pool->max) {                                                        \
                item = (T*)io_Allocator_alloc(pool->allocator, sizeof(T));                        \
                if (item) {                                                                       \
                    pool->count++;                                                                \
                }                                                                                 \
            }                                                                                     \
        }                                                                                         \
        if (item) {                                                                               \
            NAME##_list_push_back(&pool->used_list, item);                                        \
        }                                                                                         \
        return item;                                                                              \
    }                                                                                             \
                                                                                                  \
    IO_INLINE(void)                                                                               \
    NAME##_free(void* self, void* ptr)                                                            \
    {                                                                                             \
        NAME* pool = (NAME*)self;                                                                 \
        T* item = (T*)ptr;                                                                        \
        if (item) {                                                                               \
            NAME##_list_erase(&pool->used_list, item);                                            \
            NAME##_list_push_back(&pool->free_list, item);                                        \
        }                                                                                         \
    }

#endif
