#include "dx_timer.h"

bool dx_timerStart(DX_TIMER_BINDING *timer)
{
    uint64_t timeout_ms, period_ms;

    if (!timer->initialized)
    {
        uv_timer_init(uv_default_loop(), &timer->timer_handle);

        period_ms = timer->period.tv_sec * 1000;
        period_ms = period_ms + timer->period.tv_nsec / 1000000;

        if (timer->timeout != NULL)
        {
            timeout_ms = timer->timeout->tv_sec * 1000;
            timeout_ms = timeout_ms + timer->timeout->tv_nsec / 1000000;
        }
        else
        {
            timeout_ms = period_ms;
        }

        uv_timer_start(&timer->timer_handle, timer->handler, timeout_ms, period_ms);

        timer->initialized = true;
    }
    return true;
}

void dx_timerSetStart(DX_TIMER_BINDING *timerSet[], size_t timerCount)
{
    for (int i = 0; i < timerCount; i++)
    {
        if (!dx_timerStart(timerSet[i]))
        {
            break;
        };
    }
}

void dx_timerStop(DX_TIMER_BINDING *timer)
{
    if (timer->initialized)
    {
        uv_timer_stop(&timer->timer_handle);
        timer->initialized = false;
    }
}

void dx_timerSetStop(DX_TIMER_BINDING *timerSet[], size_t timerCount)
{
    for (int i = 0; i < timerCount; i++)
    {
        dx_timerStop(timerSet[i]);
    }
}

bool dx_timerStateSet(DX_TIMER_BINDING *timer, bool state){
    if (state){
        return dx_timerStart(timer);
    } else {
        dx_timerStop(timer);
        return true;
    }
} 

void dx_timerEventLoopStop(void)
{
    uv_loop_close(uv_default_loop());
}

bool dx_timerOneShotSet(DX_TIMER_BINDING *timer, const struct timespec *period)
{
    if (timer->initialized)
    {
        int64_t period_ms = period->tv_sec * 1000;
        period_ms = period_ms + period->tv_nsec / 1000000;

        uv_timer_start(&timer->timer_handle, timer->handler, period_ms, 0);

        return true;
    }
    return false;
}

int ConsumeEventLoopTimerEvent(EventLoopTimer *eventLoopTimer)
{
    return 0;
}
