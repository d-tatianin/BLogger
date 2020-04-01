#pragma once

#include <iostream>

#include "blogger/Core.h"

namespace bl {
    enum class color
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
        default = reset
    };

    class StdoutColor
    {
    public:
        static void set_to(color c)
        {
          #ifdef _WIN32
            static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);
            static bool defaults_set = false;
            static WORD default_color;

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
                SetConsoleTextAttribute(console, to_native_color(c));
          #else
            BLOGGER_COUT << to_native_color(c);
          #endif
        }

        static void reset()
        {
            set_to(color::default);
        }

        friend std::ostream& operator<<(std::ostream& stream, color c)
        {
          #ifdef _WIN32
            set_to(c);
          #else
            stream << to_native_color(c);
          #endif

            return stream;
        }
    private:
      #ifdef _WIN32
        using color_t = WORD;
      #else
        using color_t = char_t*;
      #endif

        static color_t to_native_color(color c)
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

            switch (c)
            {
            case color::black:   return black;
            case color::red:     return red;
            case color::orange:  return orange;
            case color::blue:    return blue;
            case color::green:   return green;
            case color::cyan:    return cyan;
            case color::magenta: return magenta;
            case color::yellow:  return yellow;
            case color::white:   return white;
            default:             return reset;
            }
        }
    };

    class ScopedStdoutColor
    {
    public:
        ScopedStdoutColor(color c)
        {
            StdoutColor::set_to(c);
        }

        ~ScopedStdoutColor()
        {
            StdoutColor::reset();
        }
    };
}
