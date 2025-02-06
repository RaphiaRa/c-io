/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_ASSERT_H
#define IO_ASSERT_H

#include <stdio.h>
#include <stdlib.h>

#ifndef NDEBUG
#define IO_ASSERT(cond, msg)                                                      \
    do {                                                                          \
        if (!(cond)) {                                                            \
            fprintf(stderr, "%s: %s at %s:%d\n", msg, #cond, __FILE__, __LINE__); \
            abort();                                                              \
        }                                                                         \
    } while (0)
#else
#define IO_ASSERT(cond, msg) ((void)0)
#endif

#define IO_REQUIRE(cond, msg)                                                     \
    do {                                                                          \
        if (!(cond)) {                                                            \
            fprintf(stderr, "%s: %s at %s:%d\n", msg, #cond, __FILE__, __LINE__); \
            abort();                                                              \
        }                                                                         \
    } while (0)

#endif
