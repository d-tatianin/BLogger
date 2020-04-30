#pragma once

#include <iostream>

#include "blogger/core.h"

namespace bl {
    class color
    {
    public:
      #ifdef _WIN32
        using color_t = WORD;
      #else
        using color_t = const char_t*;
      #endif

        enum type
        {
            black,
            red,
            orange,
            blue,
            green,
            cyan,
            magenta,
            yellow,
            white,
            reset,
            original = reset
        };

      #ifdef _WIN32
       // unscoped enum is intended
       #pragma warning(push)
       #pragma warning(disable:26812)
      #endif
        constexpr color(type k) noexcept
            : m_Color(k)
        {
        }

        color_t to_native() const noexcept
        {
          #ifdef _WIN32
            constexpr color_t black   = 0;
            constexpr color_t red     = 4;
            constexpr color_t orange  = 6;
            constexpr color_t blue    = 9;
            constexpr color_t green   = 10;
            constexpr color_t cyan    = 11;
            constexpr color_t magenta = 13;
            constexpr color_t yellow  = 14;
            constexpr color_t white   = 15;
            constexpr color_t reset   = -1;
          #else
            constexpr color_t black   = BLOGGER_WIDEN_IF_NEEDED("\033[0;30m");
            constexpr color_t red     = BLOGGER_WIDEN_IF_NEEDED("\033[1;31m");
            constexpr color_t orange  = BLOGGER_WIDEN_IF_NEEDED("\033[0;33m");
            constexpr color_t blue    = BLOGGER_WIDEN_IF_NEEDED("\033[1;34m");
            constexpr color_t green   = BLOGGER_WIDEN_IF_NEEDED("\033[1;32m");
            constexpr color_t cyan    = BLOGGER_WIDEN_IF_NEEDED("\033[1;36m");
            constexpr color_t magenta = BLOGGER_WIDEN_IF_NEEDED("\033[1;35m");
            constexpr color_t yellow  = BLOGGER_WIDEN_IF_NEEDED("\033[1;33m");
            constexpr color_t white   = BLOGGER_WIDEN_IF_NEEDED("\033[1;37m");
            constexpr color_t reset   = BLOGGER_WIDEN_IF_NEEDED("\033[0m");
          #endif

            switch (m_Color)
            {
                case type::black:   return black;
                case type::red:     return red;
                case type::orange:  return orange;
                case type::blue:    return blue;
                case type::green:   return green;
                case type::cyan:    return cyan;
                case type::magenta: return magenta;
                case type::yellow:  return yellow;
                case type::white:   return white;
                default:            return reset;
            }
        }
      #ifdef _WIN32
        #pragma warning(pop)
      #endif
        friend bool operator==(color l, color r) noexcept
        {
            return static_cast<int>(l.m_Color) == static_cast<int>(r.m_Color);
        }
    private:
        type m_Color;
    };

    template<ostream& stream>
    class console_color
    {
    public:
        static void set_to(color c)
        {
          #ifdef _WIN32
            static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            static bool defaults_set = false;
            static color::color_t default_color;

            if (!defaults_set)
            {
                CONSOLE_SCREEN_BUFFER_INFO console_defaults;
                GetConsoleScreenBufferInfo(console, &console_defaults);
                default_color = console_defaults.wAttributes;
                defaults_set = true;
            }

            if (c == color::reset)
                SetConsoleTextAttribute(console, default_color);
            else
                SetConsoleTextAttribute(console, c.to_native());
          #else
            stream << c.to_native();
          #endif
        }

        static void reset()
        {
            set_to(color::original);
        }
    };

    template<ostream& stream>
    class scoped_console_color
    {
    public:
        scoped_console_color(color c)
        {
            console_color<stream>::set_to(c);
        }

        ~scoped_console_color()
        {
            console_color<stream>::reset();
        }
    };

    inline ostream& operator<<(ostream& stream, color c)
    {
      #ifdef _WIN32
        // Windows doesn't differentiate between
        // STD_OUTPUT_HANDLE and STD_ERROR_HANDLE color attributes,
        // so we just set the STD_OUTPUT_HERE
        // Thanks, Windows! :(
        console_color<BLOGGER_COUT>::set_to(c);
      #else
        stream << c.to_native();
      #endif

        return stream;
    }

    inline ostream& operator<<(ostream& stream, color::type c)
    {
        return operator<<(stream, color(c));
    }
}
