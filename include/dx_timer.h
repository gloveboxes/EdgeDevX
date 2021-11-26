#pragma once

#include "dx_terminate.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <uv.h>

typedef struct uv_timer_s EventLoopTimer;
typedef struct timespec timespec;

typedef struct
{
    void (*handler)(uv_timer_t *handle);
    struct timespec period;
    struct timespec *timeout;
    const char *name;
    bool initialized;
    uv_timer_t timer_handle;
} DX_TIMER_BINDING;

bool dx_timerChange(DX_TIMER_BINDING *timer, const struct timespec *period);
bool dx_timerOneShotSet(DX_TIMER_BINDING *timer, const struct timespec *delay);
bool dx_timerStart(DX_TIMER_BINDING *timer);
bool dx_timerStateSet(DX_TIMER_BINDING *timer, bool state);
void dx_timerEventLoopStop(void);
void dx_timerSetStart(DX_TIMER_BINDING *timerSet[], size_t timerCount);
void dx_timerSetStop(DX_TIMER_BINDING *timerSet[], size_t timerCount);
void dx_timerStop(DX_TIMER_BINDING *timer);

int ConsumeEventLoopTimerEvent(EventLoopTimer *eventLoopTimer);