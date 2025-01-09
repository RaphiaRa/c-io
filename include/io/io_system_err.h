#ifndef IO_SYSTEM_ERR_H
#define IO_SYSTEM_ERR_H

#include <io/io_err.h>

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

#endif
