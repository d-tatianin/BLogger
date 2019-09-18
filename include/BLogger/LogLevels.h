#pragma once

#include "OS/Colors.h"

enum class level : int
{
    trace = 0,
    debug,
    info,
    warn,
    error,
    crit
};

inline const char* LevelToString(level lvl)
{
    switch (lvl)
    {
    case level::trace:  return "TRACE";
    case level::debug:  return "DEBUG";
    case level::info:   return "INFO";
    case level::warn:   return "WARNING";
    case level::error:  return "ERROR";
    case level::crit:   return "CRITICAL";
    default:            return nullptr;
    }
}

#define BLOGGER_TRACE_COLOR BLOGGER_WHITE
#define BLOGGER_DEBUG_COLOR BLOGGER_GREEN
#define BLOGGER_INFO_COLOR  BLOGGER_BLUE
#define BLOGGER_WARN_COLOR  BLOGGER_YELLOW
#define BLOGGER_ERROR_COLOR BLOGGER_RED
#define BLOGGER_CRIT_COLOR  BLOGGER_MAGENTA
