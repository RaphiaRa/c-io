#ifndef IO_HASH_H
#define IO_HASH_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include <io/config.h>

/** io_hash_bytes
 * @brief Fowler-Noll-Vo hash function (FNV-1a).
 * See https://en.wikipedia.org/wiki/Fowler%E2%80%93Noll%E2%80%93Vo_hash_function
 */
IO_INLINE(uint32_t)
io_hash_bytes(const void* data, size_t len)
{
    uint32_t hash = 2166136261u;
    const uint8_t* bytes = (const uint8_t*)data;
    for (size_t i = 0; i < len; ++i) {
        hash ^= bytes[i];
        hash *= 16777619;
    }
    return hash;
}

IO_INLINE(uint32_t)
io_hash_cstr(const char* str)
{
    return io_hash_bytes(str, strlen(str));
}

#endif
