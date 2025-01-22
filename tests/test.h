#ifndef IO_TEST_H
#define IO_TEST_H

#include <stdio.h>

typedef enum {
    IO_TEST_SUCCESS = 0,
    IO_TEST_FAILURE = -1,
} io_test_result;

#define IO_CHECK(x)                                                    \
    if ((x) == 0) {                                                    \
        printf("Test failed: %s, at %s:%d\n", #x, __FILE__, __LINE__); \
        return IO_TEST_FAILURE;                                        \
    }

/** io_test_setup
 * @brief Setup the test environment. This function is called before any test
 * cases are run. and does the following:
 * - Initializes the test_allocator and sets it as the default allocator.
 */
void io_test_setup(void);

void io_test_teardown(void);

/** io_TestAllocator_outstanding
 * @brief Check if there are outstanding allocations.
 * @return The number of outstanding allocations.
 */
int io_TestAllocator_outstanding(void);

#define IO_TEST_BEGIN(name)                         \
    int tests_##name(int argc, char** argv) \
    {                                               \
        (void)argc;                                 \
        (void)argv;

#define IO_TEST_END         \
    return IO_TEST_SUCCESS; \
    }

#define IO_TEST_CASE_BEGIN(name) \
    {                            \
        printf("Running test-case: %40s", #name);

#define IO_TEST_CASE_END                                                \
    int outstanding = io_TestAllocator_outstanding();                  \
    if (outstanding != 0) {                                             \
        printf(" Memory leak detected: %d allocations\n", outstanding); \
        return IO_TEST_FAILURE;                                         \
    }                                                                   \
    printf(" passed\n");                                                \
    }

#endif
