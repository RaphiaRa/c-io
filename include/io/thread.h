/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_THREAD_H
#define IO_THREAD_H

#include <io/config.h>
#include <io/err.h>

#if IO_WITH_THREADS

#include <io/assert.h>

#include <errno.h>
#include <pthread.h>

typedef struct io_Mutex {
    pthread_mutex_t mtx;
} io_Mutex;

IO_INLINE(io_Err)
io_Mutex_init(io_Mutex* mtx)
{
    if (pthread_mutex_init(&mtx->mtx, NULL) != 0) {
        return io_SystemErr(errno);
    }
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Mutex_deinit(io_Mutex* mtx)
{
    int err = pthread_mutex_destroy(&mtx->mtx);
    (void)err;
    IO_ASSERT(err == 0, "Failed to destroy mutex");
}

IO_INLINE(void)
io_Mutex_lock(io_Mutex* mtx)
{
    int err = pthread_mutex_lock(&mtx->mtx);
    (void)err;
    IO_ASSERT(err == 0, "Failed to lock mutex");
}

IO_INLINE(void)
io_Mutex_unlock(io_Mutex* mtx)
{
    int err = pthread_mutex_unlock(&mtx->mtx);
    (void)err;
    IO_ASSERT(err == 0, "Failed to unlock mutex");
}

#else

typedef struct io_Mutex {
    int dummy;
} io_Mutex;

IO_INLINE(io_Err)
io_Mutex_init(io_Mutex* mtx)
{
    (void)mtx;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Mutex_deinit(io_Mutex* mtx)
{
    (void)mtx;
}

IO_INLINE(void)
io_Mutex_lock(io_Mutex* mtx)
{
    (void)mtx;
}

IO_INLINE(void)
io_Mutex_unlock(io_Mutex* mtx)
{
    (void)mtx;
}

#endif
#endif
