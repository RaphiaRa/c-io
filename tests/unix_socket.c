#include "test.h"

#include <io/unix_socket.h>

#if IO_OS_POSIX

static void
read_callback(void* user, size_t size, io_Err err)
{
    (void)size;
    *((io_Err*)user) = err;
}

IO_TEST_BEGIN(unix_socket)
{
    IO_TEST_CASE_BEGIN(unix_socket_init)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == IO_ERR_OK);
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_init_socket_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.socket = socket_stub_emfile;
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == io_SystemErr(IO_EMFILE));
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_init_connect_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.connect = connect_stub_ebadf;
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == io_SystemErr(IO_EBADF));
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_read)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == IO_ERR_OK);
        char buf[1024];
        size_t count = sizeof(buf);
        IO_CHECK(io_UnixSocket_read(&socket, buf, &count) == IO_ERR_OK);
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_read_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == IO_ERR_OK);
        char buf[1024];
        size_t count = sizeof(buf);
        io_mock_system_call.read = read_stub_ebadf;
        IO_CHECK(io_UnixSocket_read(&socket, buf, &count) == io_SystemErr(IO_EBADF));
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_async_read)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == IO_ERR_OK);
        char buf[1024];
        size_t count = sizeof(buf);
        io_Err err = IO_ERR_OK;
        IO_CHECK(io_UnixSocket_async_read(&socket, buf, count, read_callback, &err) == IO_ERR_OK);
        io_Context_run(&ctx);
        IO_CHECK(err == IO_ERR_OK);
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_socket_async_read_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixSocket socket;
        IO_CHECK(io_UnixSocket_init(&socket, &ctx, "/test") == IO_ERR_OK);
        char buf[1024];
        size_t count = sizeof(buf);
        io_Err err = IO_ERR_OK;
        io_mock_system_call.read = read_stub_ebadf;
        IO_CHECK(io_UnixSocket_async_read(&socket, buf, count, read_callback, &err) == IO_ERR_OK);
        io_Context_run(&ctx);
        IO_CHECK(err == io_SystemErr(IO_EBADF));
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
}
IO_TEST_END

#endif
