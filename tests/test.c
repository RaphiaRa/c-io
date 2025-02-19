#include "test.h"
#include "syscall_stubs.h"

#include <io/allocator.h>
#include <io/system_call.h>

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
    io_TestAllocator* allocator = (io_TestAllocator*)test_allocator();
    int count = 0;
    for (io_AllocList* node = allocator->list; node != NULL; node = node->next)
        ++count;
    return count;
}

io_Allocator* test_allocator(void)
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
    return &allocator.base;
}

void io_test_setup(void)
{
}

void io_test_teardown(void)
{
}

io_MockSystemCall io_mock_system_call = {
    .read = read_stub_success,
    .write = write_stub_success,
    .close = close_stub_success,
    .socket = socket_stub_success,
    .bind = bind_stub_success,
    .listen = listen_stub_success,
    .accept = accept_stub_success,
    .connect = connect_stub_success,
    .pipe = pipe_stub_success,
    .fcntl = fcntl_stub_success,
    .poll = poll_stub_success,
};

void reset_system_call_stubs(void)
{
    io_mock_system_call.read = read_stub_success;
    io_mock_system_call.write = write_stub_success;
    io_mock_system_call.close = close_stub_success;
    io_mock_system_call.socket = socket_stub_success;
    io_mock_system_call.bind = bind_stub_success;
    io_mock_system_call.listen = listen_stub_success;
    io_mock_system_call.accept = accept_stub_success;
    io_mock_system_call.connect = connect_stub_success;
    io_mock_system_call.pipe = pipe_stub_success;
    io_mock_system_call.fcntl = fcntl_stub_success;
    io_mock_system_call.poll = poll_stub_success;
}
