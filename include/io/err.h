/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_ERR_H
#define IO_ERR_H

#include <io/config.h>

#include <stdio.h>
#include <stdbool.h>

typedef struct io_ErrCategory {
    const char* name;
    const char* (*msg)(int code);
} io_ErrCategory;

IO_INLINE(const char*)
io_ErrCategory_msg(const io_ErrCategory* category, int code)
{
    return category->msg(code);
}

IO_INLINE(const char*)
io_ErrCategory_name(const io_ErrCategory* category)
{
    return category->name;
}

typedef struct io_Err {
    int code;
    const io_ErrCategory* category;
} io_Err;

static size_t s = sizeof(io_Err);

#define IO_ERR_OK ((io_Err){0, NULL})

IO_INLINE(bool)
io_ok(io_Err err)
{
    return err.code == 0;
}

IO_INLINE(io_Err)
io_Err_make(int code, const io_ErrCategory* category)
{
    return (io_Err){code, category};
}

IO_INLINE(const char*)
io_Err_msg(io_Err err)
{
    return io_ErrCategory_msg(err.category, err.code);
}

IO_INLINE(int)
io_Err_printf(FILE* file, io_Err err)
{
    return fprintf(file, "Error: %s - %s\n", io_ErrCategory_name(err.category), io_Err_msg(err));
}

#endif
