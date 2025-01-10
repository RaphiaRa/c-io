#ifndef IO_LIST
#define IO_LIST

#include <io/assert.h>
#include <io/config.h>

/** Generic doubly linked list implementation.
 * that works with any struct that has a next and prev pointer.
 */
#define IO_DEFINE_LIST(NAME, T, PREV, NEXT)                                       \
    typedef struct NAME {                                                         \
        T* head;                                                                  \
        T* tail;                                                                  \
    } NAME;                                                                       \
                                                                                  \
    IO_INLINE(void)                                                               \
    NAME##_push_back(NAME* list, T* item) IO_MAYBE_UNUSED;                        \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_pop_front(NAME* list) IO_MAYBE_UNUSED;                                 \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_front(NAME* list) IO_MAYBE_UNUSED;                                     \
                                                                                  \
    IO_INLINE(void)                                                               \
    NAME##_erase(NAME* list, T* item) IO_MAYBE_UNUSED;                            \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_next(T* item) IO_MAYBE_UNUSED;                                         \
                                                                                  \
    IO_INLINE(void)                                                               \
    NAME##_push_back(NAME* list, T* item)                                         \
    {                                                                             \
        IO_ASSERT(item != NULL, "item must not be NULL");                         \
        if (list->head == NULL) {                                                 \
            list->head = item;                                                    \
            item->PREV = NULL;                                                    \
        } else {                                                                  \
            list->tail->NEXT = item;                                              \
            item->PREV = list->tail;                                              \
        }                                                                         \
        list->tail = item;                                                        \
        item->NEXT = NULL;                                                        \
    }                                                                             \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_pop_front(NAME* list)                                                  \
    {                                                                             \
        T* item = list->head;                                                     \
        if (item) {                                                               \
            list->head = item->NEXT;                                              \
            if (list->head) {                                                     \
                list->head->PREV = NULL;                                          \
            } else {                                                              \
                list->tail = NULL;                                                \
            }                                                                     \
            item->NEXT = NULL;                                                    \
        }                                                                         \
        return item;                                                              \
    }                                                                             \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_front(NAME* list)                                                      \
    {                                                                             \
        return list->head;                                                        \
    }                                                                             \
                                                                                  \
    IO_INLINE(void)                                                               \
    NAME##_erase(NAME* list, T* item)                                             \
    {                                                                             \
        IO_ASSERT(item != NULL, "item must not be NULL");                         \
        IO_ASSERT((item->NEXT || item == list->tail), "Item is not in the list"); \
        IO_ASSERT((item->PREV || item == list->head), "Item is not in the list"); \
        T* next = item->NEXT;                                                     \
        T* prev = item->PREV;                                                     \
        if (prev) {                                                               \
            prev->NEXT = next;                                                    \
            item->PREV = NULL;                                                    \
        } else {                                                                  \
            list->head = next;                                                    \
        }                                                                         \
        if (next) {                                                               \
            next->PREV = prev;                                                    \
            item->NEXT = NULL;                                                    \
        } else {                                                                  \
            list->tail = prev;                                                    \
        }                                                                         \
    }                                                                             \
                                                                                  \
    IO_INLINE(T*)                                                                 \
    NAME##_next(T* item)                                                          \
    {                                                                             \
        return item->NEXT;                                                        \
    }

#endif
