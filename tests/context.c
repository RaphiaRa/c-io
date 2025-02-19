#include "test.h"

#include <io/context.h>

IO_TEST_BEGIN(context)
{
    IO_TEST_CASE_BEGIN(context_init)
    {
        io_Err err = IO_ERR_OK;
        io_Context context;
        IO_CHECK((err = io_Context_init(&context, test_allocator())) == IO_ERR_OK);
        io_Context_deinit(&context);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(context_run)
    {
        io_Err err = IO_ERR_OK;
        io_Context context;
        IO_CHECK((err = io_Context_init(&context, test_allocator())) == IO_ERR_OK);
        io_Context_run(&context);
        io_Context_deinit(&context);
    }
    IO_TEST_CASE_END
}
IO_TEST_END
