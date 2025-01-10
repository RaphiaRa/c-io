#ifndef IO_TIMER_H
#define IO_TIMER_H

#include <io/io_assert.h>
#include <io/io_config.h>
#include <io/io_err.h>
#include <io/io_system_err.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct io_Duration {
    int seconds;
} io_Duration;

io_Duration io_seconds(int seconds);
io_Duration io_minutes(int minutes);
io_Duration io_hours(int hours);
io_Duration io_days(int days);

typedef struct io_Timer {
    time_t expire;
} io_Timer;

IO_INLINE(void)
io_Timer_init(io_Timer* timer);

IO_INLINE(io_Err)
io_Timer_set(io_Timer* timer, io_Duration duration);

IO_INLINE(bool)
io_Timer_expired(io_Timer* timer);

#ifdef IO_OS_POSIX
#include <errno.h>
#include <time.h>
#elif IO_OS_WINDOWS
#include <windows.h>
#endif

IO_INLINE(void)
io_Timer_init(io_Timer* timer)
{
    timer->expire = 0;
}

IO_INLINE(io_Err)
io_Timer_monotonic_now(time_t* out)
{
#if IO_OS_POSIX
    struct timespec ts = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0) {
        return io_SystemErr_make(errno);
    }
    *out = ts.tv_sec;
    return IO_ERR_OK;
#elif IO_OS_WINDOWS
    (void)out;
    return io_SystemErr_make(IO_ENOTSUP);
#endif
}

IO_INLINE(io_Err)
io_Timer_set(io_Timer* timer, io_Duration duration)
{
    time_t now = 0;
    io_Err err = io_Timer_monotonic_now(&now);
    IO_ASSERT(!IO_ERR_HAS(err), "io_Timer_monotonic_now failed");
    if (IO_ERR_HAS(err))
        return err;
    timer->expire = now + duration.seconds;
    return IO_ERR_OK;
}

IO_INLINE(bool)
io_Timer_expired(io_Timer* timer)
{
    time_t now = 0;
    io_Err err = io_Timer_monotonic_now(&now);
    IO_ASSERT(!IO_ERR_HAS(err), "io_Timer_monotonic_now failed");
    /* We don't return the error here, as it's already handled in io_Timer_set
     * and we can safely assume that the error won't happen here. */
    if (IO_ERR_HAS(err))
        return true;
    return now >= timer->expire;
}

#endif
