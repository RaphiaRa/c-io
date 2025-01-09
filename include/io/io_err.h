#ifndef IO_ERR_H
#define IO_ERR_H

#include <io/io_config.h>

#include <stdio.h>

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

#define IO_ERR_OK ((io_Err){0, NULL})
#define IO_ERR_HAS(lh) ((lh).code != 0)

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
    return fprintf(file, "%s: %s\n", io_ErrCategory_name(err.category), io_Err_msg(err));
}

#endif
