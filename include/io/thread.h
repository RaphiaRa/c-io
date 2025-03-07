/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_THREAD_H
#define IO_THREAD_H

#include <io/config.h>
#include <io/err.h>

typedef void* (*io_ThreadFunc)(void* user_data);

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

typedef struct io_Cond {
    pthread_cond_t cond;
} io_Cond;

IO_INLINE(io_Err)
io_Cond_init(io_Cond* cond)
{
    if (pthread_cond_init(&cond->cond, NULL) != 0) {
        return io_SystemErr(errno);
    }
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Cond_deinit(io_Cond* cond)
{
    int err = pthread_cond_destroy(&cond->cond);
    (void)err;
    IO_ASSERT(err == 0, "Failed to destroy condition variable");
}

IO_INLINE(void)
io_Cond_wait(io_Cond* cond, io_Mutex* mtx)
{
    int err = pthread_cond_wait(&cond->cond, &mtx->mtx);
    (void)err;
    IO_ASSERT(err == 0, "Failed to wait on condition variable");
}

IO_INLINE(void)
io_Cond_signal(io_Cond* cond)
{
    int err = pthread_cond_signal(&cond->cond);
    (void)err;
    IO_ASSERT(err == 0, "Failed to signal condition variable");
}

typedef struct io_Thread {
    pthread_t thread;
} io_Thread;

IO_INLINE(io_Err)
io_Thread_init(io_Thread* thread, io_ThreadFunc func, void* user_data)
{
    int err = pthread_create(&thread->thread, NULL, func, user_data);
    if (err != 0) {
        return io_SystemErr(err);
    }
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Thread_deinit(io_Thread* thread)
{
    int err = pthread_join(thread->thread, NULL);
    (void)err;
    IO_ASSERT(err == 0, "Failed to join thread");
}

typedef struct io_ThisThreadData {
    pthread_key_t key;
} io_ThisThreadData;

IO_INLINE(io_Err)
io_ThisThreadData_init(io_ThisThreadData* this_thread)
{
    if (pthread_key_create(&this_thread->key, NULL) != 0) {
        return io_SystemErr(errno);
    }
    return IO_ERR_OK;
}

IO_INLINE(void)
io_ThisThreadData_deinit(io_ThisThreadData* this_thread)
{
    int err = pthread_key_delete(this_thread->key);
    (void)err;
    IO_ASSERT(err == 0, "Failed to delete thread specific data key");
}

IO_INLINE(void)
io_ThisThreadData_set(io_ThisThreadData* this_thread, void* data)
{
    pthread_setspecific(this_thread->key, data);
}

IO_INLINE(void*)
io_ThisThreadData_get(io_ThisThreadData* this_thread)
{
    return pthread_getspecific(this_thread->key);
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

typedef struct io_Cond {
    int dummy;
} io_Cond;

IO_INLINE(io_Err)
io_Cond_init(io_Cond* cond)
{
    (void)cond;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Cond_deinit(io_Cond* cond)
{
    (void)cond;
}

IO_INLINE(void)
io_Cond_wait(io_Cond* cond, io_Mutex* mtx)
{
    (void)cond;
    (void)mtx;
}

IO_INLINE(void
io_Cond_signal(io_Cond* cond)
{
    (void)cond;
}

typedef struct io_Thread {
    int dummy;
} io_Thread;

IO_INLINE(io_Err)
io_Thread_init(io_Thread* thread, io_ThreadFunc func, void* user_data)
{
    (void)thread;
    (void)func;
    (void)user_data;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_Thread_deinit(io_Thread* thread)
{
    (void)thread;
}

typedef struct io_ThisThreadData {
    void* data;
} io_ThisThreadData;

IO_INLINE(io_Err)
io_ThisThreadData_init(io_ThisThreadData* this_thread)
{
    this_thread->data = NULL;
    return IO_ERR_OK;
}

IO_INLINE(void)
io_ThisThreadData_deinit(io_ThisThreadData* this_thread)
{
    (void)this_thread;
}

IO_INLINE(void)
io_ThisThreadData_set(io_ThisThreadData* this_thread, void* data)
{
    this_thread->data = data;
}

IO_INLINE(void*)
io_ThisThreadData_get(io_ThisThreadData* this_thread)
{
    return this_thread->data;
}

#endif
#endif
