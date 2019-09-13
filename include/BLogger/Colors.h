#pragma once

#include <iostream>

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
    #define BLOGGER_BLACK   "\033[0;30m"
    #define BLOGGER_RED     "\033[0;31m"
    #define BLOGGER_ORANGE  "\033[0;33m"
    #define BLOGGER_BLUE    "\033[0;34m"
    #define BLOGGER_GREEN   "\033[0;32m"
    #define BLOGGER_CYAN    "\033[0;36m"
    #define BLOGGER_MAGENTA "\033[0;35m"
    #define BLOGGER_YELLOW  "\033[1;33m"
    #define BLOGGER_WHITE   "\033[0;37m"
    #define BLOGGER_RESET   "\033[0m"
    #define BLOGGER_DEFAULT BLOGGER_RESET
#endif

#ifdef _WIN32

    // make windows.h less evil by default
    #ifndef BLOGGER_FULL_WINDOWS
        #define NOMINMAX
        #define WIN32_MEAN_AND_LEAN
    #endif
    #include <Windows.h>

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
    typedef const char* blogger_color;

    inline void set_output_color(blogger_color color)
    {
        std::cout << color;
    }
#endif
