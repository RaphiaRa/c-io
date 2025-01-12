#ifndef IO_TIMER_H
#define IO_TIMER_H

#include <io/assert.h>
#include <io/config.h>
#include <io/err.h>
#include <io/system_err.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef struct io_Duration {
    int ms;
} io_Duration;

IO_INLINE(io_Duration)
io_Seconds(int seconds)
{
    return (io_Duration){.ms = 1000 * seconds};
}

IO_INLINE(io_Duration)
io_Minutes(int minutes)
{
    return io_Seconds(60 * minutes);
}

IO_INLINE(io_Duration)
io_Hours(int hours)
{
    return io_Minutes(60 * hours);
}

IO_INLINE(int)
io_Duration_to_seconds(io_Duration duration)
{
    return duration.ms / 1000;
}

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
    IO_ASSERT(io_ok(err), "io_Timer_monotonic_now failed");
    if (!io_ok(err))
        return err;
    timer->expire = now + io_Duration_to_seconds(duration);
    return IO_ERR_OK;
}

IO_INLINE(bool)
io_Timer_expired(io_Timer* timer)
{
    time_t now = 0;
    io_Err err = io_Timer_monotonic_now(&now);
    IO_ASSERT(io_ok(err), "io_Timer_monotonic_now failed");
    /* We don't return the error here, as it's already handled in io_Timer_set
     * and we can safely assume that the error won't happen here. */
    if (!io_ok(err))
        return true;
    return now >= timer->expire;
}

#endif
