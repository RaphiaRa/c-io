#include "test.h"
#include <io/allocator.h>

#include <assert.h>
#include <stdlib.h>

typedef struct io_AllocList {
    void* ptr;
    struct io_AllocList* next;
} io_AllocList;

typedef struct io_TestAllocator {
    io_Allocator base;
    io_AllocList* list;
} io_TestAllocator;

static void*
io_TestAllocator_alloc(void* self, size_t size)
{
    io_TestAllocator* allocator = self;
    void* ptr = calloc(1, size);
    if (!ptr)
        return NULL;
    io_AllocList* node = calloc(1, sizeof(io_AllocList));
    if (!node) {
        free(ptr);
        return NULL;
    }
    node->ptr = ptr;
    node->next = allocator->list;
    allocator->list = node;
    return ptr;
}

static void*
io_TestAllocator_realloc(void* self, void* ptr, size_t size)
{
    io_TestAllocator* allocator = self;
    for (io_AllocList* node = allocator->list; node != NULL; node = node->next) {
        if (node->ptr == ptr) {
            void* new_ptr = realloc(ptr, size);
            if (!new_ptr)
                return NULL;
            node->ptr = new_ptr;
            return new_ptr;
        }
    }
    return io_TestAllocator_alloc(self, size);
}

static void
io_TestAllocator_free(void* self, void* ptr)
{
    io_TestAllocator* allocator = self;
    io_AllocList* prev = NULL;
    for (io_AllocList* node = allocator->list; node != NULL; node = node->next) {
        if (node->ptr == ptr) {
            if (prev)
                prev->next = node->next;
            else
                allocator->list = node->next;
            free(node->ptr);
            free(node);
            return;
        }
        prev = node;
    }
    assert(0 && "io_TestAllocator_free: invalid pointer");
}

int io_TestAllocator_outstanding(void)
{
    io_TestAllocator* allocator = (io_TestAllocator*)io_DefaultAllocator();
    int count = 0;
    for (io_AllocList* node = allocator->list; node != NULL; node = node->next)
        ++count;
    return count;
}

void io_test_setup(void)
{
    static io_AllocatorMethods methods = {
        .alloc = io_TestAllocator_alloc,
        .realloc = io_TestAllocator_realloc,
        .free = io_TestAllocator_free,
    };
    static io_TestAllocator allocator = {
        .base.methods = &methods,
        .list = NULL,
    };
    io_set_allocator(&allocator.base);
}

void io_test_teardown(void)
{
    io_TestAllocator* allocator = (io_TestAllocator*)io_DefaultAllocator();
    for (io_AllocList* node = allocator->list; node != NULL;) {
        io_AllocList* next = node->next;
        free(node->ptr);
        free(node);
        node = next;
    }
    allocator->list = NULL;
    io_set_allocator(NULL);
}
