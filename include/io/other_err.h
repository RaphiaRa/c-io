#ifndef IO_OTHER_ERR_H
#define IO_OTHER_ERR_H

#include <io/err.h>

#include <errno.h>
#include <string.h>

typedef enum io_OtherErrc {
    IO_OTHER_ERRC_EOF = 1,
    IO_OTHER_ERRC_UNKNOWN = 2,
} io_OtherErrc;

IO_INLINE(io_ErrCategory*)
io_OtherErrCategory(void);

IO_INLINE(const char*)
io_OtherErr_msg(int code)
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

IO_INLINE(io_Err)
io_OtherErr_make(int code)
{
    return (io_Err){.code = code, .category = io_OtherErrCategory()};
}

IO_INLINE(io_ErrCategory*)
io_OtherErrCategory(void)
{
    static io_ErrCategory category = {
        .name = "other",
        .msg = io_OtherErr_msg,
    };
    return &category;
}

#define IO_ERR_EOF ((io_Err){IO_OTHER_ERRC_EOF, io_OtherErrCategory()})
#define IO_ERR_UNKNOWN ((io_Err){IO_OTHER_ERRC_UNKNOWN, io_OtherErrCategory()})
#endif
