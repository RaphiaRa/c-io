/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_ERR_H
#define IO_ERR_H

#include <io/assert.h>
#include <io/basic_err.h>
#include <io/config.h>
#include <io/other_err.h>
#include <io/system_err.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#define IO_ERR_OK ((io_Err)0)

IO_INLINE(const char*)
io_Err_msg(io_Err err)
{
    switch (IO_ERR_CATEGORY(err)) {
    case IO_SYSTEM_CATEGORY:
        return io_SystemErr_msg(IO_ERR_CODE(err));
    case IO_OTHER_CATEGORY:
        return io_OtherErr_msg(IO_ERR_CODE(err));
    default:
        return "Unknown";
    }
}

IO_INLINE(const char*)
io_Err_category_name(io_Err err)
{
    switch (IO_ERR_CATEGORY(err)) {
    case IO_SYSTEM_CATEGORY:
        return "System";
    case IO_OTHER_CATEGORY:
        return "Other";
    default:
        return "Unknown";
    }
}

IO_INLINE(int)
io_Err_printf(FILE* file, io_Err err)
{
    return fprintf(file, "Error: %s - %s\n", io_Err_category_name(err), io_Err_msg(err));
}

#endif
