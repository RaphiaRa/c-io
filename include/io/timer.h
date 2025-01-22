/*
 * SPDX-FileCopyrightText: 2025 c-io Contributers
 *
 * SPDX-License-Identifier: MPL-2.0
 */

#ifndef IO_TIMER_H
#define IO_TIMER_H

#include <io/assert.h>
#include <io/config.h>
#include <io/err.h>
#include <io/system_err.h>
#include <io/utility.h>

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

typedef enum io_Duration {
    IO_DURATION_TYPE
} io_Duration;

IO_INLINE(io_Duration)
io_Seconds(int seconds)
{
    return (io_Duration)(seconds * 1000);
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
    return (int)duration / 1000;
}

IO_INLINE(int)
io_Duration_to_ms(io_Duration duration)
{
    return (int)duration;
}

typedef struct io_Timer {
    time_t expire;
} io_Timer;

IO_INLINE(void)
io_Timer_init(io_Timer* timer, io_Duration duration);

IO_INLINE(void)
io_Timer_set(io_Timer* timer, io_Duration duration);

IO_INLINE(io_Duration)
io_Timer_remaining(const io_Timer* timer);

IO_INLINE(bool)
io_Timer_expired(const io_Timer* timer);

#ifdef IO_OS_POSIX
#include <errno.h>
#include <time.h>
#elif IO_OS_WINDOWS
#include <windows.h>
#endif

IO_INLINE(void)
io_Timer_init(io_Timer* timer, io_Duration duration)
{
    timer->expire = 0;
    io_Timer_set(timer, duration);
}

IO_INLINE(io_Err)
io_TimerImpl_monotonic_now(time_t* out)
{
#if IO_OS_POSIX
    struct timespec ts = {0};
    int ret = clock_gettime(CLOCK_MONOTONIC, &ts);
    if (ret != 0) {
        return io_SystemErr(errno);
    }
    *out = ts.tv_sec;
    return IO_ERR_OK;
#elif IO_OS_WINDOWS
    (void)out;
    return io_SystemErr(IO_ENOTSUP);
#endif
}

IO_INLINE(void)
io_Timer_set(io_Timer* timer, io_Duration duration)
{
    time_t now = 0;
    io_Err err = io_TimerImpl_monotonic_now(&now);
    IO_REQUIRE(!err, "io_Timer_monotonic_now failed");
    timer->expire = now + io_Duration_to_seconds(duration);
}

IO_INLINE(io_Duration)
io_Timer_remaining(const io_Timer* timer)
{
    time_t now = 0;
    io_Err err = io_TimerImpl_monotonic_now(&now);
    IO_REQUIRE(!err, "io_Timer_monotonic_now failed");
    return io_Seconds(IO_MAX((int)(timer->expire - now), 0));
}

IO_INLINE(bool)
io_Timer_expired(const io_Timer* timer)
{
    return io_Timer_remaining(timer) == (io_Duration)(0);
}

IO_INLINE(bool)
io_Timer_less(const io_Timer* a, const io_Timer* b)
{
    return a->expire < b->expire;
}

#endif
