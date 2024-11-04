#pragma once

#include "basic_ronin.h"

#include "Debug.h"

enum LogLevel
{
    Backend,
    Frontend
};

extern LogLevel rloglevel;

void ronin_log(const char *msg);

void ronin_warn(const char *msg);

void ronin_err(const char *msg);

void ronin_err_d(const char *msg);

void ronin_err_sdl2();
