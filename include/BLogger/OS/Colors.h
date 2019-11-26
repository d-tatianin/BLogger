#pragma once

#include <iostream>

#include "BLogger/Core.h"

#ifdef _WIN32
    #define BLOGGER_BLACK   0
    #define BLOGGER_RED     4
    #define BLOGGER_ORANGE  6
    #define BLOGGER_BLUE    9
    #define BLOGGER_GREEN   10
    #define BLOGGER_CYAN    11
    #define BLOGGER_MAGENTA 13
    #define BLOGGER_YELLOW  14
    #define BLOGGER_WHITE   15
    #define BLOGGER_RESET   0xffff
    #define BLOGGER_DEFAULT BLOGGER_RESET
#else
    #define BLOGGER_BLACK   BLOGGER_MAKE_UNICODE("\033[0;30m")
    #define BLOGGER_RED     BLOGGER_MAKE_UNICODE("\033[1;31m")
    #define BLOGGER_ORANGE  BLOGGER_MAKE_UNICODE("\033[0;33m")
    #define BLOGGER_BLUE    BLOGGER_MAKE_UNICODE("\033[1;34m")
    #define BLOGGER_GREEN   BLOGGER_MAKE_UNICODE("\033[1;32m")
    #define BLOGGER_CYAN    BLOGGER_MAKE_UNICODE("\033[1;36m")
    #define BLOGGER_MAGENTA BLOGGER_MAKE_UNICODE("\033[1;35m")
    #define BLOGGER_YELLOW  BLOGGER_MAKE_UNICODE("\033[1;33m")
    #define BLOGGER_WHITE   BLOGGER_MAKE_UNICODE("\033[1;37m")
    #define BLOGGER_RESET   BLOGGER_MAKE_UNICODE("\033[0m")
    #define BLOGGER_DEFAULT BLOGGER_RESET
#endif

#ifdef _WIN32
    typedef WORD blogger_color;

    inline void set_output_color(blogger_color color)
    {
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

        if (color == BLOGGER_RESET)
            SetConsoleTextAttribute(console, default_color);
        else
            SetConsoleTextAttribute(console, color);
    }
#else
    typedef const bl_char* blogger_color;

    inline void set_output_color(blogger_color color)
    {
        BLOGGER_COUT << color;
    }
#endif
