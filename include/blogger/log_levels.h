#pragma once

#include "os/colors.h"
#include "core.h"


namespace bl {
    class level
    {
    public:
        enum type
        {
            trace,
            debug,
            info,
            warn,
            error,
            crit
        };

        static constexpr color trace_color = color::white;
        static constexpr color debug_color = color::green;
        static constexpr color info_color  = color::blue;
        static constexpr color warn_color  = color::yellow;
        static constexpr color error_color = color::red;
        static constexpr color crit_color  = color::magenta;

      #ifdef _WIN32
        // unscoped enum is intended
        #pragma warning(push)
        #pragma warning(disable:26812)
      #endif
        constexpr level(type t) noexcept
            : m_level(t)
        {
        }

        const char_t* to_string() const noexcept
        {
            switch (m_level)
            {
                case type::trace: return BLOGGER_WIDEN_IF_NEEDED("TRACE");
                case type::debug: return BLOGGER_WIDEN_IF_NEEDED("DEBUG");
                case type::info:  return BLOGGER_WIDEN_IF_NEEDED("INFO");
                case type::warn:  return BLOGGER_WIDEN_IF_NEEDED("WARNING");
                case type::error: return BLOGGER_WIDEN_IF_NEEDED("ERROR");
                case type::crit:  return BLOGGER_WIDEN_IF_NEEDED("CRITICAL");
                default:          return BLOGGER_WIDEN_IF_NEEDED("UNKNOWN");
            }
        }
      #ifdef _WIN32
        #pragma warning(pop)
      #endif

        color to_color() const noexcept
        {
            switch (m_level)
            {
                case type::trace:  return trace_color;
                case type::debug:  return debug_color;
                case type::info:   return info_color;
                case type::warn:   return warn_color;
                case type::error:  return error_color;
                case type::crit:   return crit_color;
                default:           return bl::color::reset;
            }
        }

        friend bool operator>(level l, level r) noexcept
        {
            return static_cast<int>(l.m_level) > static_cast<int>(r.m_level);
        }

        friend bool operator<(level l, level r) noexcept
        {
            return static_cast<int>(l.m_level) < static_cast<int>(r.m_level);
        }

        friend bool operator==(level l, level r) noexcept
        {
            return static_cast<int>(l.m_level) == static_cast<int>(r.m_level);
        }
    private:
        type m_level;
    };

    inline ostream& operator<<(ostream& stream, level l) noexcept
    {
        return stream << l.to_string();
    }

    inline ostream& operator<<(ostream& stream, level::type l) noexcept
    {
        return operator<<(stream, level(l));
    }
}
