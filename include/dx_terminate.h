#pragma once

#include "dx_exit_codes.h"
#include <signal.h>
#include <stdbool.h>
#include <string.h>
#include <uv.h>

bool dx_isTerminationRequired(void);
int dx_getTerminationExitCode(void);
void dx_registerTerminationHandler(void);
void dx_runEventLoop(void);
void dx_terminate(int exitCode);
void dx_terminationHandler(int signalNumber);