/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_GAI_ERR_H
#define IO_GAI_ERR_H

#include <io/assert.h>
#include <io/basic_err.h>
#include <io/config.h>
#include <io/utility.h>

#include <netdb.h>
#include <string.h>

IO_INLINE(const char*)
io_GaiErr_msg(uint32_t code)
{
    return gai_strerror((int)code);
}

#define IO_GAI_CATEGORY IO_FOURCC('G', 'A', 'I', '_')

IO_INLINE(io_Err)
io_GaiErr(int code)
{
    IO_ASSERT(code > 0, "Invalid gai error code");
    return IO_ERR_PACK(IO_GAI_CATEGORY, (uint32_t)code);
}

#endif
