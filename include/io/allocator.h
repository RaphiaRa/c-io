/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_ALLOCATOR_H
#define IO_ALLOCATOR_H

#include <stddef.h>
#include <stdlib.h>

#include <io/config.h>

typedef struct io_AllocatorMethods {
    void* (*alloc)(void* self, size_t size);
    void* (*realloc)(void* self, void* ptr, size_t size);
    void (*free)(void* self, void* ptr);
} io_AllocatorMethods;

typedef struct io_Allocator {
    io_AllocatorMethods* methods;
} io_Allocator;

IO_INLINE(void*)
io_Allocator_alloc(io_Allocator* allocator, size_t size)
{
    return allocator->methods->alloc(allocator, size);
}

IO_INLINE(void*)
io_Allocator_realloc(io_Allocator* allocator, void* ptr, size_t size)
{
    return allocator->methods->realloc(allocator, ptr, size);
}

IO_INLINE(void)
io_Allocator_free(io_Allocator* allocator, void* ptr)
{
    allocator->methods->free(allocator, ptr);
}

IO_INLINE(void*)
io_SystemAllocator_alloc(void* self, size_t size)
{
    (void)self;
    return malloc(size);
}

IO_INLINE(void*)
io_SystemAllocator_realloc(void* self, void* addr, size_t size)
{
    (void)self;
    return realloc(addr, size);
}

IO_INLINE(void)
io_SystemAllocator_free(void* self, void* addr)
{
    (void)self;
    free(addr);
}

IO_INLINE(io_Allocator*)
io_SystemAllocator(void)
{
    static io_AllocatorMethods methods = {
        .alloc = io_SystemAllocator_alloc,
        .realloc = io_SystemAllocator_realloc,
        .free = io_SystemAllocator_free,
    };
    static io_Allocator allocator = {
        .methods = &methods,
    };
    return &allocator;
}

IO_INLINE(void*)
io_alloc(io_Allocator* allocator, size_t size)
{
    return io_Allocator_alloc(allocator, size);
}

IO_INLINE(void*)
io_realloc(io_Allocator* allocator, void* addr, size_t size)
{
    return io_Allocator_realloc(allocator, addr, size);
}

IO_INLINE(void)
io_free(io_Allocator* allocator, void* addr)
{
    io_Allocator_free(allocator, addr);
}

#endif
