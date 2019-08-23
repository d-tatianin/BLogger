#pragma once

#include <iostream>

#ifdef _WIN32
    #define BLOGGER_BLACK   0
    #define BLOGGER_BLUE    9
    #define BLOGGER_GREEN   10
    #define BLOGGER_CYAN    11
    #define BLOGGER_RED     12
    #define BLOGGER_MAGENTA 13
    #define BLOGGER_YELLOW  14
    #define BLOGGER_WHITE   15
    #define BLOGGER_RESET   99
#else
    #define BLOGGER_BLACK   "\033[0;30m"
    #define BLOGGER_RED     "\033[0;31m"
    #define BLOGGER_GREEN   "\033[0;32m"
    #define BLOGGER_YELLOW  "\033[0;33m"
    #define BLOGGER_BLUE    "\033[0;34m"
    #define BLOGGER_MAGENTA "\033[0;35m"
    #define BLOGGER_CYAN    "\033[0;36m"
    #define BLOGGER_WHITE   "\033[0;37m"
    #define BLOGGER_RESET   "\033[0m"
#endif

#ifdef _WIN32
    #include <Windows.h>

    typedef WORD blogger_color;

    inline void set_output_color(blogger_color color)
    {
        static HANDLE console = GetStdHandle(STD_OUTPUT_HANDLE);

        static CONSOLE_SCREEN_BUFFER_INFO console_defaults;
        static bool defaults_set = false;

        if (!defaults_set)
        {
            GetConsoleScreenBufferInfo(console, &console_defaults);
            defaults_set = true;
        }

        if (color == BLOGGER_RESET)
            SetConsoleTextAttribute(console, console_defaults.wAttributes);
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
