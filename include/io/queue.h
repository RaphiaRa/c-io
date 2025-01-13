/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_QUEUE_H
#define IO_QUEUE_H

#include <io/config.h>

/** Generic queue implementation.
 * that works with any struct that has a next pointer.
 */
#define IO_DEFINE_QUEUE(NAME, T)                                 \
    typedef struct NAME {                                        \
        T* head;                                                 \
        T* tail;                                                 \
    } NAME;                                                      \
                                                                 \
    IO_INLINE(NAME)                                              \
    NAME##_make(void) IO_MAYBE_UNUSED;                           \
                                                                 \
    IO_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item) IO_MAYBE_UNUSED;           \
                                                                 \
    IO_INLINE(T*)                                                \
    NAME##_pop(NAME* queue) IO_MAYBE_UNUSED;                     \
                                                                 \
    IO_INLINE(bool)                                              \
    NAME##_empty(NAME* queue) IO_MAYBE_UNUSED;                   \
                                                                 \
    IO_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other) IO_MAYBE_UNUSED; \
                                                                 \
    IO_INLINE(NAME)                                              \
    NAME##_make(void)                                            \
    {                                                            \
        return (NAME){.head = NULL, .tail = NULL};               \
    }                                                            \
                                                                 \
    IO_INLINE(bool)                                              \
    NAME##_empty(NAME* queue)                                    \
    {                                                            \
        return queue->head == NULL;                              \
    }                                                            \
                                                                 \
    IO_INLINE(void)                                              \
    NAME##_push(NAME* queue, T* item)                            \
    {                                                            \
        if (queue->head == NULL) {                               \
            queue->head = item;                                  \
        } else {                                                 \
            queue->tail->next = item;                            \
        }                                                        \
        queue->tail = item;                                      \
        item->next = NULL;                                       \
    }                                                            \
                                                                 \
    IO_INLINE(void)                                              \
    NAME##_push_queue(NAME* queue, NAME* other)                  \
    {                                                            \
        if (queue->head == NULL) {                               \
            *queue = *other;                                     \
        } else if (other->head) {                                \
            queue->tail->next = other->head;                     \
            queue->tail = other->tail;                           \
        }                                                        \
        *other = NAME##_make();                                  \
    }                                                            \
                                                                 \
    IO_INLINE(T*)                                                \
    NAME##_pop(NAME* queue)                                      \
    {                                                            \
        T* item = queue->head;                                   \
        if (item) {                                              \
            queue->head = item->next;                            \
            item->next = NULL;                                   \
        }                                                        \
        return item;                                             \
    }

#endif
