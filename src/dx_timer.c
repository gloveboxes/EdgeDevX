#include "dx_timer.h"

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;



bool dx_timerChange(DX_TIMER_BINDING *timer, const struct timespec *repeat)
{
    pthread_mutex_lock(&mutex);

    if (!timer->initialized)
    {
        pthread_mutex_unlock(&mutex);
        return false;
    }

    uint64_t timer_ms = repeat->tv_sec * 1000;
    timer_ms = timer_ms + repeat->tv_nsec / 1000000;
    uv_timer_set_repeat(&timer->timer_handle, timer_ms);

    pthread_mutex_unlock(&mutex);
    return true;
}

bool dx_timerStart(DX_TIMER_BINDING *timer)
{
    pthread_mutex_lock(&mutex);

    if (!timer->initialized) {
        if (timer->delay && timer->repeat) {
            printf("Can't specify both a timer delay and a repeat period for timer %s\n", timer->name);
            dx_terminate(DX_ExitCode_Create_Timer_Failed);

            pthread_mutex_unlock(&mutex);
            return false;
        }

        uv_timer_init(uv_default_loop(), &timer->timer_handle);

        if (timer->delay) {
            uint64_t timer_ms = timer->delay->tv_sec * 1000;
            timer_ms = timer_ms + timer->delay->tv_nsec / 1000000;
            uv_timer_start(&timer->timer_handle, timer->handler, timer_ms, 0);

        } else if (timer->repeat) {
            uint64_t timer_ms = timer->repeat->tv_sec * 1000;
            timer_ms = timer_ms + timer->repeat->tv_nsec / 1000000;
            uv_timer_start(&timer->timer_handle, timer->handler, timer_ms, timer_ms);
        }

        timer->initialized = true;
    }

    pthread_mutex_unlock(&mutex);
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

bool dx_timerStateSet(DX_TIMER_BINDING *timer, bool state)
{
    if (state)
    {
        return dx_timerStart(timer);
    }
    else
    {
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
    pthread_mutex_lock(&mutex);
    bool result = false;

    if (timer->initialized)
    {
        int64_t period_ms = period->tv_sec * 1000;
        period_ms = period_ms + period->tv_nsec / 1000000;

        uv_update_time(uv_default_loop());

        uv_timer_start(&timer->timer_handle, timer->handler, period_ms, 0);

        result = true;
    }

    pthread_mutex_unlock(&mutex);
    return result;
}

int ConsumeEventLoopTimerEvent(EventLoopTimer *eventLoopTimer)
{
    return 0;
}

void dx_eventLoopRun(void)
{
    uv_run(uv_default_loop(), UV_RUN_DEFAULT);
}

void dx_eventLoopStop(void)
{
    uv_stop(uv_default_loop());
}
