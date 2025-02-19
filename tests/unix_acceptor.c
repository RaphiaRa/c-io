#include "test.h"

#include <io/unix_acceptor.h>

#if IO_OS_POSIX

static void
accept_callback(void* user, io_Err err)
{
    *((io_Err*)user) = err;
}

IO_TEST_BEGIN(unix_acceptor)
{
    IO_TEST_CASE_BEGIN(unix_acceptor_init)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == IO_ERR_OK);
        io_UnixAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_acceptor_init_socket_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.socket = socket_stub_emfile;
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == io_SystemErr(IO_EMFILE));
        io_UnixAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_acceptor_init_bind_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.bind = bind_stub_eaddrinuse;
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == io_SystemErr(IO_EADDRINUSE));
        io_UnixAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_acceptor_init_fcntl_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.fcntl = fcntl_stub_einval;
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == io_SystemErr(IO_EINVAL));
        io_UnixAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_acceptor_accept)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == IO_ERR_OK);
        io_UnixSocket socket;
        io_UnixSocket_init(&socket, &ctx, NULL);
        IO_CHECK(io_UnixAcceptor_accept(&acceptor, &socket) == IO_ERR_OK);
        io_UnixAcceptor_deinit(&acceptor);
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(unix_acceptor_async_accept)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_UnixAcceptor acceptor;
        IO_CHECK(io_UnixAcceptor_init(&acceptor, &ctx, "/test") == IO_ERR_OK);
        io_UnixSocket socket;
        io_UnixSocket_init(&socket, &ctx, NULL);
        io_Err result = IO_ERR_OK;
        IO_CHECK(io_UnixAcceptor_async_accept(&acceptor, &socket, accept_callback, &result) == IO_ERR_OK);
        io_Context_run(&ctx);
        IO_CHECK(result == IO_ERR_OK);
        io_UnixAcceptor_deinit(&acceptor);
        io_UnixSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
}
IO_TEST_END

#endif
