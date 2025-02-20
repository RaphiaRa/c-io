/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_OTHER_ERR_H
#define IO_OTHER_ERR_H

#include <io/basic_err.h>
#include <io/config.h>
#include <io/utility.h>

#include <errno.h>
#include <string.h>

typedef enum io_OtherErrc {
    IO_OTHER_ERRC_EOF = 1,
    IO_OTHER_ERRC_UNKNOWN = 2,
    IO_OTHER_ERRC_NO_ENDPOINT = 3,
} io_OtherErrc;

IO_INLINE(const char*)
io_OtherErr_msg(uint32_t code)
{
    switch (code) {
    case 0:
        return "Success";
    case IO_OTHER_ERRC_EOF:
        return "End of File";
    default:
        return "Unknown";
        break;
    }
}

#define IO_OTHER_CATEGORY IO_FOURCC('O', 'T', 'H', 'R')
#define IO_OTHER_ERR(code) IO_ERR_PACK(IO_OTHER_CATEGORY, code)

IO_INLINE(io_Err)
io_OtherErr(uint32_t code)
{
    return IO_OTHER_ERR(code);
}

#define IO_ERR_EOF (IO_OTHER_ERR(IO_OTHER_ERRC_EOF))
#define IO_ERR_UNKNOWN (IO_OTHER_ERR(IO_OTHER_ERRC_UNKNOWN))

#endif
