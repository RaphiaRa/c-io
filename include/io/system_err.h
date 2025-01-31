/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_SYSTEM_ERR_H
#define IO_SYSTEM_ERR_H

#include <io/assert.h>
#include <io/basic_err.h>
#include <io/config.h>
#include <io/utility.h>

#include <errno.h>
#include <string.h>

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
#define IO_EINVAL EINVAL
#define IO_EMFILE EMFILE
#define IO_EADDRINUSE EADDRINUSE
#define IO_EPIPE EPIPE
#elif TH_OS_WINDOWS
#define IO_ENOTSUP ERROR_NOT_SUPPORTED
#define IO_ECANCELED ERROR_CANCELLED
#define IO_EIO ERROR_IO_DEVICE
#define IO_EBADF ERROR_BAD_FORMAT
#define IO_EAGAIN ERROR_RETRY
#define IO_EWOULDBLOCK ERROR_RETRY
#define IO_ETIMEDOUT ERROR_TIMEOUT
#define IO_ENOMEM ERROR_OUTOFMEMORY
#define IO_EINVAL ERROR_INVALID_PARAMETER
#define IO_EMFILE ERROR_TOO_MANY_OPEN_FILES
#define IO_EADDRINUSE ERROR_ADDRESS_IN_USE
#define IO_EPIPE ERROR_BROKEN_PIPE
#endif

IO_INLINE(const char*)
io_SystemErr_msg(uint32_t code)
{
    return strerror((int)code);
}

#define IO_SYSTEM_CATEGORY IO_FOURCC('S', 'Y', 'S', 'T')

IO_INLINE(io_Err)
io_SystemErr(int code)
{
    IO_ASSERT(code > 0, "Invalid system error code");
    return IO_ERR_PACk(IO_SYSTEM_CATEGORY, (uint32_t)code);
}

#endif
