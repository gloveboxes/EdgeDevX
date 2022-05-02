#pragma once

#include "dx_terminate.h"
#include <event2/event.h>
#include <event2/event_struct.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

#define DX_TIMER_HANDLER(name)                                   \
    void name(evutil_socket_t fd, short event, void *arg)        \
    {                                                            \
        if (ConsumeEventLoopTimerEvent(arg) != 0)                \
        {                                                        \
            dx_terminate(DX_ExitCode_ConsumeEventLoopTimeEvent); \
            return;                                              \
        }

#define DX_TIMER_HANDLER_END }

#define DX_DECLARE_TIMER_HANDLER(name) void name(evutil_socket_t fd, short event, void *arg)

typedef struct event EventLoopTimer;
typedef struct timespec timespec;

typedef struct
{
    void (*handler)(evutil_socket_t fd, short event, void *arg);
    struct timespec *delay;
    struct timespec *repeat;
    const char *name;
    EventLoopTimer *event_handle;
    struct timeval _period;
} DX_TIMER_BINDING;

int ConsumeEventLoopTimerEvent(EventLoopTimer *eventLoopTimer);
void dx_eventLoopRun(void);
void dx_eventLoopStop(void);

bool dx_timerChange(DX_TIMER_BINDING *timer, const struct timespec *period);
bool dx_timerOneShotSet(DX_TIMER_BINDING *timer, const struct timespec *delay);
bool dx_timerStart(DX_TIMER_BINDING *timer);
bool dx_timerStateSet(DX_TIMER_BINDING *timer, bool state);
void dx_timerEventLoopStop(void);
void dx_timerSetStart(DX_TIMER_BINDING *timerSet[], size_t timerCount);
void dx_timerSetStop(DX_TIMER_BINDING *timerSet[], size_t timerCount);
void dx_timerStop(DX_TIMER_BINDING *timer);
