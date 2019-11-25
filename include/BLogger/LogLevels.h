#pragma once

#include "OS/Colors.h"
#include "Core.h"

enum class level : int
{
    trace = 0,
    debug,
    info,
    warn,
    error,
    crit
};

inline const bl_char* LevelToString(level lvl)
{
    switch (lvl)
    {
    case level::trace:  return BLOGGER_MAKE_UNICODE("TRACE");
    case level::debug:  return BLOGGER_MAKE_UNICODE("DEBUG");
    case level::info:   return BLOGGER_MAKE_UNICODE("INFO");
    case level::warn:   return BLOGGER_MAKE_UNICODE("WARNING");
    case level::error:  return BLOGGER_MAKE_UNICODE("ERROR");
    case level::crit:   return BLOGGER_MAKE_UNICODE("CRITICAL");
    default:            return nullptr;
    }
}

#define BLOGGER_TRACE_COLOR BLOGGER_WHITE
#define BLOGGER_DEBUG_COLOR BLOGGER_GREEN
#define BLOGGER_INFO_COLOR  BLOGGER_BLUE
#define BLOGGER_WARN_COLOR  BLOGGER_YELLOW
#define BLOGGER_ERROR_COLOR BLOGGER_RED
#define BLOGGER_CRIT_COLOR  BLOGGER_MAGENTA
