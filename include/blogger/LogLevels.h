#pragma once

#include "OS/Colors.h"
#include "Core.h"


namespace bl {
    enum class level
    {
        trace,
        debug,
        info,
        warn,
        error,
        crit
    };

    inline const char_t* level_to_string(level lvl)
    {
        switch (lvl)
        {
        case level::trace:  return BLOGGER_WIDEN_IF_NEEDED("TRACE");
        case level::debug:  return BLOGGER_WIDEN_IF_NEEDED("DEBUG");
        case level::info:   return BLOGGER_WIDEN_IF_NEEDED("INFO");
        case level::warn:   return BLOGGER_WIDEN_IF_NEEDED("WARNING");
        case level::error:  return BLOGGER_WIDEN_IF_NEEDED("ERROR");
        case level::crit:   return BLOGGER_WIDEN_IF_NEEDED("CRITICAL");
        default:            return nullptr;
        }
    }

    constexpr color trace_color = color::white;
    constexpr color debug_color = color::green;
    constexpr color info_color  = color::blue;
    constexpr color warn_color  = color::yellow;
    constexpr color error_color = color::red;
    constexpr color crit_color  = color::magenta;

    inline color level_to_color(level lvl)
    {
        switch (lvl)
        {
        case level::trace:  return trace_color;
        case level::debug:  return debug_color;
        case level::info:   return info_color;
        case level::warn:   return warn_color;
        case level::error:  return error_color;
        case level::crit:   return crit_color;
        default:            return bl::color::reset;
        }
    }
}
