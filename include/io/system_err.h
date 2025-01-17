/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_SYSTEM_ERR_H
#define IO_SYSTEM_ERR_H

#include <io/err.h>

#include <errno.h>
#include <string.h>

IO_INLINE(io_ErrCategory*)
io_SystemErrCategory(void);

IO_INLINE(const char*)
io_SystemErr_msg(int code)
{
    return strerror(code);
}

IO_INLINE(io_Err)
io_SystemErr_make(int code)
{
    return (io_Err){.code = code, .category = io_SystemErrCategory()};
}

IO_INLINE(io_ErrCategory*)
io_SystemErrCategory(void)
{
    static io_ErrCategory category = {
        .name = "system",
        .msg = io_SystemErr_msg,
    };
    return &category;
}

/* Define the system error codes that we use */
#if IO_OS_POSIX
#define IO_ENOTSUP ENOTSUP
#define IO_ECANCELED ECANCELED
#define IO_EIO EIO
#define IO_EBADF EBADF
#define IO_EAGAIN EAGAIN
#define IO_EWOULDBLOCK EWOULDBLOCK
#define IO_ETIMEDOUT ETIMEDOUT
#define IO_ENOMEM ENOMEM
#elif TH_OS_WINDOWS
#define IO_ENOTSUP ERROR_NOT_SUPPORTED
#define IO_ECANCELED ERROR_CANCELLED
#define IO_EIO ERROR_IO_DEVICE
#define IO_EBADF ERROR_BAD_FORMAT
#define IO_EAGAIN ERROR_RETRY
#define IO_EWOULDBLOCK ERROR_RETRY
#define IO_ETIMEDOUT ERROR_TIMEOUT
#define IO_ENOMEM ERROR_OUTOFMEMORY
#endif
#endif
