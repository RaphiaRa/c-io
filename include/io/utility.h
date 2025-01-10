#ifndef IO_UTILITY_H
#define IO_UTILITY_H

#include <io/config.h>

#include <stdlib.h>

#define IO_MIN(a, b) ((a) < (b) ? (a) : (b))
#define IO_MAX(a, b) ((a) > (b) ? (a) : (b))
#define IO_ABS(a) ((a) < 0 ? -(a) : (a))

#define IO_ARRAY_SIZE(arr) (sizeof(arr) / sizeof(arr[0]))

// Move a pointer from src to dst and set src to NULL
IO_INLINE(void*)
io_move_ptr(void** src)
{
    void* dst = *src;
    *src = NULL;
    return dst;
}

#define IO_MOVE_PTR(ptr) io_move_ptr((void**)&(ptr))

// Mathematical utility functions

IO_INLINE(size_t)
io_next_pow2(size_t n)
{
    IO_ASSERT(n > 0, "n must be larger 0");
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    n++;
    return n;
}

#endif
