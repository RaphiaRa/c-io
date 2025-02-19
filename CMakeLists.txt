cmake_minimum_required(VERSION 3.15 FATAL_ERROR)
project(c-io)

set(CMAKE_C_STANDARD 99)
include(CheckIPOSupported)
check_ipo_supported()

# Extra warnings, treat warnings as error
if(CMAKE_C_COMPILER_ID MATCHES "Clang|GNU")
   SET(IO_COMPILE_OPTIONS -Wall -Wextra -Wpedantic -Werror -Wstrict-prototypes -Wmissing-prototypes -Wshadow -Wconversion)
   message(STATUS "Using Clang/GNU compiler, use options: ${IO_COMPILE_OPTIONS}")
endif()
function (io_set_default_compile_options target)
    target_compile_options(${target} PRIVATE ${IO_COMPILE_OPTIONS})
endfunction()

add_library(io INTERFACE)
add_library(io::io ALIAS io)

# Logging
if (NOT DEFINED IO_LOG_LEVEL)
    set(IO_LOG_LEVEL IO_LOG_LEVEL_INFO)
endif()

# OpenSSL
#find_package(OpenSSL)
#if (OpenSSL_FOUND)
#    message(STATUS "Found OpenSSL: ${OPENSSL_INCLUDE_DIR}")
#    target_link_libraries(io PRIVATE OpenSSL::SSL OpenSSL::Crypto)
#    set(IO_WITH_SSL 1)
#else()
#    message(STATUS "OpenSSL not found, disabling SSL support")
#    set(IO_WITH_SSL 0)
#endif()

set_property(TARGET io PROPERTY IO_LOG_LEVEL=${TH_LOG_LEVEL})
set_property(TARGET io PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${CMAKE_CURRENT_SOURCE_DIR}/include)

if (NOT IO_DISABLE_EXAMPLES)
    function(add_io_example EXAMPLE_NAME)
        add_executable(${EXAMPLE_NAME} examples/${EXAMPLE_NAME}.c)
        target_link_libraries(${EXAMPLE_NAME} PUBLIC io::io)
        io_set_default_compile_options(${EXAMPLE_NAME})
    endfunction(add_io_example)
    add_io_example(accept)
    add_io_example(async_accept)
endif()

# Tests

if (NOT IO_DISABLE_TESTS)
    include(CMakePushCheckState)
    include(CheckCCompilerFlag)
    cmake_push_check_state(RESET)
    set(CMAKE_REQUIRED_LINK_OPTIONS -fsanitize=address)
    check_c_compiler_flag("-fsanitize=address" IO_HAVE_ASAN)
    cmake_pop_check_state()

    #if (OpenSSL_FOUND)
    #    target_link_libraries(io_test PRIVATE OpenSSL::SSL OpenSSL::Crypto)
    #endif()
    
    #target_compile_definitions(tiny_http_test PRIVATE IO_OS_MOCK=1)
    #target_compile_definitions(tiny_http_test PRIVATE IO_LOG_LEVEL=${TH_LOG_LEVEL})
    enable_testing()
    set(CMAKE_TESTDRIVER_BEFORE_TESTMAIN "io_test_setup();")
    set(CMAKE_TESTDRIVER_AFTER_TESTMAIN "io_test_teardown();")

    set(IO_TESTS
        tests/context.c
        tests/unix_acceptor.c
        tests/unix_socket.c
    )

    create_test_sourcelist(IO_TEST_SRC_LIST io_test.c
        ${IO_TESTS}
        EXTRA_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR}/tests/test.h
    )

    add_executable(io_test 
        tests/test.c 
        tests/syscall_stubs.c
        ${IO_TEST_SRC_LIST}
    )

    target_link_libraries(io_test PUBLIC 
        io::io
    )
    io_set_default_compile_options(io_test)
    if (IO_HAVE_ASAN)
        target_compile_options(io_test PUBLIC -fsanitize=address -fno-omit-frame-pointer)
        target_link_options(io_test PUBLIC -fsanitize=address -fno-omit-frame-pointer)
    else()
        message(WARNING "Address sanitizer not found, tests will not be run with ASAN")
    endif()

    # Enable mocking for testing
    target_compile_definitions(io_test PRIVATE IO_MOCKING=1)

    foreach (test_src ${IO_TESTS})
        get_filename_component(test_name ${test_src} NAME_WE)
        add_test(NAME tests/${test_name} COMMAND io_test tests/${test_name})
    endforeach()
endif()

# Amalgamation

if (NOT IO_DISABLE_AMALGAMATION)
    set(Python3_FIND_VIRTUALENV FIRST)
    find_package(Python3)
    if (Python3_FOUND)
        message(STATUS "Found Python3: ${Python3_EXECUTABLE}")
    else()
        message(STATUS "Python3 not found, disabling amalgamation")
        SET(IO_DISABLE_AMALGAMATION 1)
    endif()
endif()

# Check whether all required python modules are available

if (NOT IO_DISABLE_AMALGAMATION)
    SET(PYTHON_REQUIRED_MODULES os re argparse networkx)
    foreach (PYTHON_MODULE ${PYTHON_REQUIRED_MODULES})
        execute_process(
            COMMAND ${Python3_EXECUTABLE} -c "import ${PYTHON_MODULE}"
            RESULT_VARIABLE PYTHON_MODULE_RESULT
        )
        if (NOT PYTHON_MODULE_RESULT EQUAL 0)
            message(STATUS "Python module ${PYTHON_MODULE} not found, disabling amalgamation")
            SET(IO_DISABLE_AMALGAMATION 1)
        endif()
    endforeach()
endif()


if (NOT IO_DISABLE_AMALGAMATION)
   function(amalgamation_generate OUTPUT_FILE)
        # Amalgamation of io.h
        add_custom_command(
            OUTPUT ${OUTPUT_FILE}
            COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/amalgamate.py -i ${CMAKE_CURRENT_SOURCE_DIR}/include -o ${OUTPUT_FILE}
            COMMENT "Running amalgamation.py on ${OUTPUT_FILE}"
            WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
            VERBATIM
        )
    endfunction(amalgamation_generate OUTPUT_FILE)
    amalgamation_generate(${CMAKE_CURRENT_BINARY_DIR}/io.h)
    add_custom_target(amalgamation ALL DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/io.h)
endif()
