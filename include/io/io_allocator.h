#ifndef IO_ALLOCATOR_H
#define IO_ALLOCATOR_H

#include <stddef.h>

#include <io/io_config.h>

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

#endif
