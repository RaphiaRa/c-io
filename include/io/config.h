/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_CONFIG_H
#define IO_CONFIG_H

#define IO_MAYBE_UNUSED __attribute__((unused))
#define IO_INLINE(type) static inline type
#define IO_WITH_POLL 1
#define IO_OS_POSIX 1
#define IO_DEFAULT_TIMEOUT 10 // seconds
#endif
