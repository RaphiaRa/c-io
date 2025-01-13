/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_VEC_H
#define IO_VEC_H

#include <io/allocator.h>
#include <io/assert.h>
#include <io/config.h>
#include <io/utility.h>

#include <stddef.h>

#define IO_DEFINE_VEC(NAME, TYPE, DEINIT)                                                                  \
    typedef struct NAME {                                                                                  \
        TYPE* data;                                                                                        \
        size_t size;                                                                                       \
        size_t capacity;                                                                                   \
        io_Allocator* allocator;                                                                           \
    } NAME;                                                                                                \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, io_Allocator* allocator) IO_MAYBE_UNUSED;                                       \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec) IO_MAYBE_UNUSED;                                                               \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec) IO_MAYBE_UNUSED;                                                              \
                                                                                                           \
    IO_INLINE(size_t)                                                                                      \
    NAME##_size(const NAME* vec) IO_MAYBE_UNUSED;                                                          \
                                                                                                           \
    IO_INLINE(size_t)                                                                                      \
    NAME##_capacity(const NAME* vec) IO_MAYBE_UNUSED;                                                      \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_resize(NAME* vec, size_t size) IO_MAYBE_UNUSED;                                                 \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_push_back(NAME* vec, TYPE value) IO_MAYBE_UNUSED;                                               \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index) IO_MAYBE_UNUSED;                                                    \
                                                                                                           \
    IO_INLINE(const TYPE*)                                                                                 \
    NAME##_cat(const NAME* vec, size_t index) IO_MAYBE_UNUSED;                                             \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec) IO_MAYBE_UNUSED;                                                               \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec) IO_MAYBE_UNUSED;                                                                 \
                                                                                                           \
    IO_INLINE(TYPE)                                                                                       \
    NAME##_back(NAME* vec) IO_MAYBE_UNUSED;                                                                \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_init(NAME* vec, io_Allocator* allocator)                                                        \
    {                                                                                                      \
        vec->allocator = allocator ? allocator : io_DefaultAllocator();                                    \
        vec->capacity = 0;                                                                                 \
        vec->size = 0;                                                                                     \
        vec->data = NULL;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_deinit(NAME* vec)                                                                               \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
            io_Allocator_free(vec->allocator, vec->data);                                                  \
        }                                                                                                  \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_clear(NAME* vec)                                                                                \
    {                                                                                                      \
        if (vec->data) {                                                                                   \
            for (size_t i = 0; i < vec->size; i++) {                                                       \
                DEINIT(&vec->data[i]);                                                                     \
            }                                                                                              \
        }                                                                                                  \
        vec->size = 0;                                                                                     \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(size_t)                                                                                      \
    NAME##_size(const NAME* vec)                                                                           \
    {                                                                                                      \
        return vec->size;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(size_t)                                                                                      \
    NAME##_capacity(const NAME* vec)                                                                       \
    {                                                                                                      \
        return vec->capacity;                                                                              \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_resize(NAME* vec, size_t size)                                                                  \
    {                                                                                                      \
        if (size < vec->size) {                                                                            \
            vec->size = size;                                                                              \
            return;                                                                                        \
        }                                                                                                  \
        if (size > vec->capacity) {                                                                        \
            size_t new_capacity = io_next_pow2(size);                                                      \
            TYPE* new_data = io_Allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            IO_ASSERT(new_data, "Out of memory");                                                          \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->size = size;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(void)                                                                                        \
    NAME##_push_back(NAME* vec, TYPE value)                                                                \
    {                                                                                                      \
        if (vec->size >= vec->capacity) {                                                                  \
            size_t new_capacity = vec->capacity == 0 ? 1 : vec->capacity * 2;                              \
            TYPE* new_data = io_Allocator_realloc(vec->allocator, vec->data, new_capacity * sizeof(TYPE)); \
            IO_ASSERT(new_data, "Out of memory");                                                          \
            vec->data = new_data;                                                                          \
            vec->capacity = new_capacity;                                                                  \
        }                                                                                                  \
        vec->data[vec->size++] = value;                                                                    \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_at(NAME* vec, size_t index)                                                                     \
    {                                                                                                      \
        IO_ASSERT(index <= vec->size, "index exceeds vector size");                                        \
        return vec->data + index;                                                                          \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(const TYPE*)                                                                                 \
    NAME##_cat(const NAME* vec, size_t index)                                                              \
    {                                                                                                      \
        IO_ASSERT(index <= vec->size, "index exceeds vector size");                                        \
        return (const TYPE*)&vec->data[index];                                                                          \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_begin(NAME* vec)                                                                                \
    {                                                                                                      \
        return vec->data;                                                                                  \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(TYPE*)                                                                                       \
    NAME##_end(NAME* vec)                                                                                  \
    {                                                                                                      \
        return vec->data + vec->size;                                                                      \
    }                                                                                                      \
                                                                                                           \
    IO_INLINE(TYPE)                                                                                        \
    NAME##_back(NAME* vec)                                                                                 \
    {                                                                                                      \
        return vec->data[vec->size - 1];                                                                   \
    }

#endif
