#ifndef IO_ASSERT_H
#define IO_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#ifndef NDEBUG
#define IO_ASSERT(cond, msg)                                                      \
    do {                                                                          \
        if (!(cond)) {                                                            \
            fprintf(stderr, "msg: %s at %s:%d\n", msg, #cond, __FILE__, __LINE__); \
            abort();                                                              \
        }                                                                         \
    } while (0)
#else
#define IO_ASSERT(cond) ((void)0)
#endif

#define IO_REQUIRE(cond, msg)                                                    \
    do {                                                                         \
        if (!(cond)) {                                                           \
            fprintf(stderr, "%s: %s at %s:%d\n", msg, #cond, __FILE__, __LINE__); \
            abort();                                                             \
        }                                                                        \
    } while (0)

#endif
