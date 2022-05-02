#include "dx_timer.h"

static bool initialized = false;
static struct event_base *base;

static void init_event_loop(void)
{
	if (!initialized)
	{
		base        = event_base_new();
		initialized = true;
	}
}

bool dx_timerChange(DX_TIMER_BINDING *timer, const struct timespec *repeat)
{
	if (!timer->event_handle)
	{
		return false;
	}

	event_del(timer->event_handle);

	timer->_period.tv_sec  = repeat->tv_sec;
	timer->_period.tv_usec = repeat->tv_nsec / 1000;

	int result = event_add(timer->event_handle, &timer->_period);

	return true;
}

bool dx_timerStart(DX_TIMER_BINDING *timer)
{
	if (!timer->event_handle)
	{
		if (timer->delay != NULL && timer->repeat != NULL)
		{
			printf("Can't specify both a timer delay and a repeat period for timer %s\n", timer->name);
			dx_terminate(DX_ExitCode_Create_Timer_Failed);
			return false;
		}

		init_event_loop();

		if (timer->delay != NULL)
		{
			timer->event_handle = event_new(base, -1, 0, timer->handler, (void *)timer->event_handle);
			if (timer->event_handle)
			{
				timer->_period.tv_sec  = timer->delay->tv_sec;
				timer->_period.tv_usec = timer->delay->tv_nsec / 1000;

				event_add(timer->event_handle, &timer->_period);
			}
			else
			{
				printf("oneshot event_new failed\n");
				dx_terminate(DX_ExitCode_Create_Timer_Failed);
			}
		}
		else if (timer->repeat != NULL)
		{
			timer->event_handle =
				event_new(base, -1, EV_PERSIST, timer->handler, (void *)timer->event_handle);
			if (timer->event_handle)
			{
				timer->_period.tv_sec  = timer->repeat->tv_sec;
				timer->_period.tv_usec = timer->repeat->tv_nsec / 1000;

				event_add(timer->event_handle, &timer->_period);
			}
			else
			{
				printf("repeat event_new failed\n");
				dx_terminate(DX_ExitCode_Create_Timer_Failed);
			}
		}
		else
		{
			timer->event_handle = event_new(base, -1, 0, timer->handler, (void *)timer->event_handle);
			if (!timer->event_handle)
			{
				printf("oneshot event_new failed\n");
				dx_terminate(DX_ExitCode_Create_Timer_Failed);
			}
		}
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
	if (timer->event_handle != NULL)
	{
		event_free(timer->event_handle);
		timer->event_handle = NULL;
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
	if (!base)
	{
		event_base_free(base);
	}
}

bool dx_timerOneShotSet(DX_TIMER_BINDING *timer, const struct timespec *period)
{
	if (timer->event_handle != NULL && period != NULL)
	{
		timer->_period.tv_sec  = period->tv_sec;
		timer->_period.tv_usec = period->tv_nsec / 1000;

		if (event_add(timer->event_handle, &timer->_period))
		{
			printf("Error setting oneshot timer\n");
		}

		return true;
	}
	return false;
}

int ConsumeEventLoopTimerEvent(EventLoopTimer *eventLoopTimer)
{
	return 0;
}

void dx_eventLoopRun(void)
{
	event_base_dispatch(base);
}

void dx_eventLoopStop(void)
{
	event_base_loopexit(base, NULL);
}
