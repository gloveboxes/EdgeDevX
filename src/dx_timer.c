#include "dx_timer.h"

static bool initialized = false;
static struct event_base *base;
static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

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
	pthread_mutex_lock(&mutex);

	if (!timer->event_handle)
	{
		pthread_mutex_unlock(&mutex);
		return false;
	}

	event_del(timer->event_handle);

	int result = event_add(timer->event_handle, &(struct timeval){repeat->tv_sec, repeat->tv_nsec / 1000});

	pthread_mutex_unlock(&mutex);
	return true;
}

bool dx_timerStart(DX_TIMER_BINDING *timer)
{
	pthread_mutex_lock(&mutex);

	if (!timer->event_handle)
	{
		if (timer->delay && timer->repeat)
		{
			printf("Can't specify both a timer delay and a repeat period for timer %s\n", timer->name);
			dx_terminate(DX_ExitCode_Create_Timer_Failed);
			
			pthread_mutex_unlock(&mutex);
			return false;
		}

		init_event_loop();

		if (timer->delay)
		{
			timer->event_handle = event_new(base, -1, 0, timer->handler, (void *)timer->event_handle);
			if (timer->event_handle)
			{
				event_add(timer->event_handle, &(struct timeval){timer->delay->tv_sec, timer->delay->tv_nsec / 1000});
			}
			else
			{
				printf("oneshot event_new failed\n");
				dx_terminate(DX_ExitCode_Create_Timer_Failed);
			}
		}
		else if (timer->repeat)
		{
			timer->event_handle =
				event_new(base, -1, EV_PERSIST, timer->handler, (void *)timer->event_handle);
			if (timer->event_handle)
			{
				event_add(timer->event_handle, &(struct timeval){timer->repeat->tv_sec, timer->repeat->tv_nsec / 1000});
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
	if (timer->event_handle)
	{
		event_del(timer->event_handle);
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
	if (base)
	{
		event_base_free(base);
	}
}

bool dx_timerOneShotSet(DX_TIMER_BINDING *timer, const struct timespec *period)
{
	bool result = false;
	pthread_mutex_lock(&mutex);
	
	if (timer->event_handle && period)
	{
		if (event_add(timer->event_handle, &(struct timeval){period->tv_sec, period->tv_nsec / 1000}))
		{
			printf("Error setting oneshot timer\n");
			result = false;
		} else {
			result = true;
		}		
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
	event_base_dispatch(base);
}

void dx_eventLoopStop(void)
{
	event_base_loopexit(base, NULL);
}
