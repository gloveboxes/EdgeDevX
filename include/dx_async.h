#pragma once

#include "dx_terminate.h"
#include <stdbool.h>
#include <uv.h>

#define DX_ASYNC_HANDLER(name, handle) \
	void name(uv_async_t *handle)      \
	{

#define DX_ASYNC_HANDLER_END }

#define DX_DECLARE_TIMER_ASYNC_HANDLER(name) void name(uv_async_t *handle)

typedef struct
{
	uv_async_t async;
	void (*handler)(uv_async_t *handle);
} DX_ASYNC_BINDING;

void dx_asyncInit(DX_ASYNC_BINDING *async);
void dx_asyncSend(DX_ASYNC_BINDING *binding, void *data);
void dx_asyncSetInit(DX_ASYNC_BINDING *asyncSet[], size_t asyncCount);