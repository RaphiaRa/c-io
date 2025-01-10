#ifndef IO_CONTEXT_H
#define IO_CONTEXT_H

#include <stddef.h>

#include <io/io_allocator.h>
#include <io/io_assert.h>
#include <io/io_queue.h>
#include <io/io_loop.h>

typedef struct io_Context {
    io_Loop loop;
    io_Allocator* allocator;
} io_Context;

IO_INLINE(io_Err)
io_Context_init(io_Context* context, io_Allocator* allocator)
{
    (void)context;
    (void)allocator;
    return IO_ERR_OK;
}

#endif
