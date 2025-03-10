#include "test.h"

#include <io/tcp_socket.h>

// static void
// read_callback(void* user, size_t size, io_Err err)
//{
//     (void)size;
//     *((io_Err*)user) = err;
// }

IO_TEST_BEGIN(tcp_socket)
{
    IO_TEST_CASE_BEGIN(tcp_socket_init)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_TcpSocket socket;
        IO_CHECK(io_TcpSocket_init(&socket, &ctx, "localhost:8080") == IO_ERR_OK);
        io_TcpSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_socket_init_socket_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.socket = socket_stub_emfile;
        io_TcpSocket socket;
        IO_CHECK(io_TcpSocket_init(&socket, &ctx, "localhost:8080") == io_OtherErr(IO_OTHER_ERRC_NO_ENDPOINT));
        io_TcpSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
    IO_TEST_CASE_BEGIN(tcp_socket_init_connect_fail)
    {
        io_Context ctx;
        io_Context_init(&ctx, test_allocator());
        io_mock_system_call.connect = connect_stub_ebadf;
        io_TcpSocket socket;
        IO_CHECK(io_TcpSocket_init(&socket, &ctx, "localhost:8080") == io_OtherErr(IO_OTHER_ERRC_NO_ENDPOINT));
        io_TcpSocket_deinit(&socket);
        io_Context_deinit(&ctx);
    }
    IO_TEST_CASE_END
}
IO_TEST_END
