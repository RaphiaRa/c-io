/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_HASHMAP_H
#define IO_HASHMAP_H

#include <io/allocator.h>
#include <io/assert.h>
#include <io/config.h>
#include <io/hash.h>
#include <io/system_err.h>

#include <stddef.h>
#include <string.h>

#define IO_DEFINE_HASHMAP(NAME, K, V, HASH, K_EQ, K_NULL)                                                                           \
    typedef struct NAME##_entry {                                                                                                   \
        K key;                                                                                                                      \
        V value;                                                                                                                    \
    } NAME##_entry;                                                                                                                 \
                                                                                                                                    \
    typedef struct NAME {                                                                                                           \
        NAME##_entry* entries;                                                                                                      \
        size_t size;                                                                                                                \
        size_t capacity;                                                                                                            \
        size_t end;                                                                                                                 \
        size_t begin;                                                                                                               \
        io_Allocator* allocator;                                                                                                    \
    } NAME;                                                                                                                         \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, io_Allocator* allocator) IO_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map) IO_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    IO_INLINE(io_Err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity) IO_MAYBE_UNUSED;                                                                     \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map) IO_MAYBE_UNUSED;                                                                                       \
                                                                                                                                    \
    IO_INLINE(io_Err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value) IO_MAYBE_UNUSED;                                                                          \
                                                                                                                                    \
    IO_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key) IO_MAYBE_UNUSED;                                                                         \
                                                                                                                                    \
    typedef NAME##_entry* NAME##_iter;                                                                                              \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key) IO_MAYBE_UNUSED;                                                                            \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry) IO_MAYBE_UNUSED;                                                                   \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map) IO_MAYBE_UNUSED;                                                                                        \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry) IO_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry) IO_MAYBE_UNUSED;                                                                    \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_init(NAME* map, io_Allocator* allocator)                                                                                 \
    {                                                                                                                               \
        map->allocator = allocator;                                                                                                 \
        if (map->allocator == NULL) {                                                                                               \
            map->allocator = io_DefaultAllocator();                                                                                 \
        }                                                                                                                           \
        map->entries = NULL;                                                                                                        \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_deinit(NAME* map)                                                                                                        \
    {                                                                                                                               \
        if (map->entries) {                                                                                                         \
            io_Allocator_free(map->allocator, map->entries);                                                                        \
        }                                                                                                                           \
        map->entries = NULL;                                                                                                        \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_reset(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->entries && map->size > 0) {                                                                                        \
            for (size_t i = map->begin; i < map->end; i++) {                                                                        \
                map->entries[i].key = K_NULL;                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        map->size = 0;                                                                                                              \
        map->capacity = 0;                                                                                                          \
        map->begin = 0;                                                                                                             \
        map->end = 0;                                                                                                               \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(io_Err)                                                                                                               \
    NAME##_reserve(NAME* map, size_t capacity)                                                                                      \
    {                                                                                                                               \
        if (map->capacity >= capacity) {                                                                                            \
            return IO_ERR_OK;                                                                                                       \
        }                                                                                                                           \
        capacity = io_next_pow2(capacity);                                                                                          \
        NAME##_entry* entries = (NAME##_entry*)io_Allocator_realloc(map->allocator, map->entries, capacity * sizeof(NAME##_entry)); \
        if (!entries) {                                                                                                             \
            return io_SystemErr(IO_ENOMEM);                                                                                         \
        }                                                                                                                           \
        for (size_t i = map->capacity; i < capacity; i++) {                                                                         \
            entries[i] = (NAME##_entry){.key = K_NULL};                                                                             \
        }                                                                                                                           \
        map->entries = entries;                                                                                                     \
        map->capacity = capacity;                                                                                                   \
        return IO_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_update_begin_end(NAME* map, size_t new_index)                                                                            \
    {                                                                                                                               \
        if (map->size == 1) {                                                                                                       \
            map->begin = new_index;                                                                                                 \
            map->end = new_index + 1;                                                                                               \
        } else {                                                                                                                    \
            if (new_index < map->begin) {                                                                                           \
                map->begin = new_index;                                                                                             \
            }                                                                                                                       \
            if (new_index + 1 > map->end) {                                                                                         \
                map->end = new_index + 1;                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_do_set(NAME* map, uint32_t hash, K key, V value)                                                                         \
    {                                                                                                                               \
        for (size_t i = hash; i < map->capacity; i++) {                                                                             \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return;                                                                                                             \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                return;                                                                                                             \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = 0; i < hash; i++) {                                                                                         \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                map->size++;                                                                                                        \
                NAME##_update_begin_end(map, i);                                                                                    \
                return;                                                                                                             \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                entry->key = key;                                                                                                   \
                entry->value = value;                                                                                               \
                return;                                                                                                             \
            }                                                                                                                       \
        }                                                                                                                           \
        IO_REQUIRE(0, "No space in map left");                                                                                      \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry) IO_MAYBE_UNUSED;                                                                \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_fix_hole(NAME* map, NAME##_entry* entry)                                                                                 \
    {                                                                                                                               \
        size_t last_zeroed = entry - map->entries;                                                                                  \
        for (size_t i = entry - map->entries + 1; i < map->end; i++) {                                                              \
            uint32_t hash = 0;                                                                                                      \
            if (K_EQ(map->entries[i].key, K_NULL)) {                                                                                \
                break;                                                                                                              \
            } else if ((hash = (HASH(map->entries[i].key) & (map->capacity - 1))) <= last_zeroed) {                                 \
                map->entries[last_zeroed] = map->entries[i];                                                                        \
                map->entries[i].key = K_NULL;                                                                                       \
                last_zeroed = i;                                                                                                    \
            }                                                                                                                       \
        }                                                                                                                           \
        if (map->size == 0) {                                                                                                       \
            map->begin = 0;                                                                                                         \
            map->end = 0;                                                                                                           \
        } else if (last_zeroed == map->end - 1) {                                                                                   \
            map->end = NAME##_prev(map, &map->entries[last_zeroed]) - map->entries + 1;                                             \
        } else if (last_zeroed == map->begin) {                                                                                     \
            map->begin = NAME##_next(map, &map->entries[last_zeroed]) - map->entries;                                               \
        }                                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(io_Err)                                                                                                               \
    NAME##_expand(NAME* map)                                                                                                        \
    {                                                                                                                               \
        size_t old_capacity = map->capacity;                                                                                        \
        size_t new_capacity = old_capacity * 2;                                                                                     \
        if (new_capacity == 0) {                                                                                                    \
            new_capacity = 1;                                                                                                       \
        }                                                                                                                           \
        io_Err err = NAME##_reserve(map, new_capacity);                                                                             \
        if (err != IO_ERR_OK) {                                                                                                     \
            return err;                                                                                                             \
        }                                                                                                                           \
        /* Need to rehash all entries */                                                                                            \
        for (size_t i = 0; i < old_capacity; i++) {                                                                                 \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                /* rearranged == 0; */                                                                                              \
                continue;                                                                                                           \
            }                                                                                                                       \
            uint32_t hash = HASH(entry->key);                                                                                       \
            /* Don't need to rehash every entry */                                                                                  \
            hash &= (new_capacity - 1);                                                                                             \
            NAME##_entry e = *entry;                                                                                                \
            entry->key = K_NULL;                                                                                                    \
            --map->size;                                                                                                            \
            NAME##_fix_hole(map, entry);                                                                                            \
            NAME##_do_set(map, hash, e.key, e.value);                                                                               \
        }                                                                                                                           \
        return IO_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(io_Err)                                                                                                               \
    NAME##_set(NAME* map, K key, V value)                                                                                           \
    {                                                                                                                               \
        if (map->size >= map->capacity / 2) {                                                                                       \
            io_Err err = NAME##_expand(map);                                                                                        \
            if (err != IO_ERR_OK) {                                                                                                 \
                return err;                                                                                                         \
            }                                                                                                                       \
        }                                                                                                                           \
        uint32_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        NAME##_do_set(map, hash, key, value);                                                                                       \
        return IO_ERR_OK;                                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_find(const NAME* map, K key)                                                                                             \
    {                                                                                                                               \
        uint32_t hash = HASH(key) & (map->capacity - 1);                                                                            \
        if (map->size == 0) {                                                                                                       \
            return NULL;                                                                                                            \
        }                                                                                                                           \
        for (size_t i = hash; i < map->end; i++) {                                                                                  \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        for (size_t i = map->begin; i < hash; i++) {                                                                                \
            NAME##_entry* entry = &map->entries[i];                                                                                 \
            if (K_EQ(entry->key, K_NULL)) {                                                                                         \
                return NULL;                                                                                                        \
            }                                                                                                                       \
            if (K_EQ(entry->key, key)) {                                                                                            \
                return entry;                                                                                                       \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(void)                                                                                                                 \
    NAME##_erase(NAME* map, NAME##_entry* entry)                                                                                    \
    {                                                                                                                               \
        entry->key = K_NULL;                                                                                                        \
        map->size--;                                                                                                                \
        NAME##_fix_hole(map, entry);                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(V*)                                                                                                                   \
    NAME##_try_get(const NAME* map, K key)                                                                                          \
    {                                                                                                                               \
        NAME##_entry* entry = NAME##_find(map, key);                                                                                \
        if (entry) {                                                                                                                \
            return &entry->value;                                                                                                   \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_begin(NAME* map)                                                                                                         \
    {                                                                                                                               \
        if (map->begin == map->end)                                                                                                 \
            return NULL;                                                                                                            \
        return &map->entries[map->begin];                                                                                           \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_next(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        size_t i = entry - map->entries;                                                                                            \
        for (size_t j = i + 1; j < map->end; j++) {                                                                                 \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NULL;                                                                                                                \
    }                                                                                                                               \
                                                                                                                                    \
    IO_INLINE(NAME##_entry*)                                                                                                        \
    NAME##_prev(NAME* map, NAME##_entry* entry)                                                                                     \
    {                                                                                                                               \
        size_t i = entry - map->entries;                                                                                            \
        for (size_t j = i - 1; j >= map->begin; j--) {                                                                              \
            NAME##_entry* e = &map->entries[j];                                                                                     \
            if (!K_EQ(e->key, K_NULL)) {                                                                                            \
                return e;                                                                                                           \
            }                                                                                                                       \
        }                                                                                                                           \
        return NAME##_begin(map);                                                                                                   \
    }

#endif
