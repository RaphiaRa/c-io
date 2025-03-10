#include "test.h"

#include <io/tcp_acceptor.h>

#if IO_OS_POSIX

static void
accept_callback(void* user, io_Err err)
{
    *((io_Err*)user) = err;
}

IO_TEST_BEGIN(tcp_acceptor)
{
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_ip4)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0:8080") == IO_ERR_OK);
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_ip6)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "[::]:8080") == IO_ERR_OK);
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_bad_addr)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "localhost:8080") == io_SystemErr(IO_EINVAL));
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_missing_port)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0") == io_SystemErr(IO_EINVAL));
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_socket_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.socket = socket_stub_emfile;
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0:8080") == io_SystemErr(IO_EMFILE));
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_init_bind_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.bind = bind_stub_eaddrinuse;
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0:8080") == io_SystemErr(IO_EADDRINUSE));
        io_TcpAcceptor_deinit(&acceptor);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_accept)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0:8080") == IO_ERR_OK);
        io_TcpSocket socket;
        io_TcpSocket_init(&socket, &ctx, NULL);
        IO_CHECK(io_TcpAcceptor_accept(&acceptor, &socket) == IO_ERR_OK);
        io_TcpAcceptor_deinit(&acceptor);
        io_TcpSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_acceptor_async_accept)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpAcceptor acceptor;
        IO_CHECK(io_TcpAcceptor_init(&acceptor, &ctx, "0.0.0.0:8080") == IO_ERR_OK);
        io_TcpSocket socket;
        io_TcpSocket_init(&socket, &ctx, NULL);
        io_Err result = IO_ERR_OK;
        IO_CHECK(io_TcpAcceptor_async_accept(&acceptor, &socket, accept_callback, &result) == IO_ERR_OK);
        io_Context_run(&ctx);
        IO_CHECK(result == IO_ERR_OK);
        io_TcpAcceptor_deinit(&acceptor);
        io_TcpSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
}
IO_TEST_END

#endif
