#pragma once

#include <utility>
#include <iostream>
#include <vector>
#include <mutex>

#ifdef _WIN32
    #ifndef BLOGGER_FULL_WINDOWS
        #define NOMINMAX
        #define WIN32_MEAN_AND_LEAN
    #endif
    #include <windows.h>
#endif

// ---- Custom BLogger types ----

#ifdef BLOGGER_UNICODE_MODE
    #include <wchar.h>
    #include <cwchar>
    typedef wchar_t bl_char;
    #define BLOGGER_MAKE_UNICODE(str) L##str
    #define BLOGGER_COUT ::std::wcout
    #ifdef _WIN32
        #define BLOGGER_FILEMODE BLOGGER_MAKE_UNICODE("w")
    #elif defined(__linux__)
        #define BLOGGER_EXTRA_BYTES 0
        #define BLOGGER_FILEMODE "w"
    #endif
    #define STRING_LENGTH(string) wcslen(string)
    #define TIME_TO_STRING(out, out_size, in_format, in_time) wcsftime(out, out_size, in_format, in_time)
    #define FORMAT_STRING(out, out_size, in_format, ...) swprintf(out, out_size, in_format, __VA_ARGS__)
    #define TO_STRING(what) ::std::to_wstring(what)
    #define COMPATIBLE_OSTREAM ::std::wostream
#else
    typedef char bl_char;
    #define BLOGGER_MAKE_UNICODE(str) str
    #define BLOGGER_COUT ::std::cout
    #define BLOGGER_FILEMODE "w"
    #define STRING_LENGTH(string) strlen(string)
    #define TIME_TO_STRING(out, out_size, in_format, in_time) strftime(out, out_size, in_format, in_time)
    #define FORMAT_STRING(out, out_size, in_format, ...) snprintf(out, out_size, in_format, __VA_ARGS__)
    #define TO_STRING(what) ::std::to_string((what))
    #define COMPATIBLE_OSTREAM ::std::ostream
#endif

#ifdef _WIN32 // CLRF?
    #define BLOGGER_TRUE_SIZE(size) static_cast<size_t>(size) + 3
#elif defined(__linux__)
    #define BLOGGER_TRUE_SIZE(size) size
#endif

typedef std::basic_string<bl_char, std::char_traits<bl_char>> BLoggerString;
typedef std::basic_stringstream<bl_char, std::char_traits<bl_char>> BLoggerStringStream;
typedef std::vector<bl_char> bl_string;
using internal_buffer = bl_string;
typedef internal_buffer BLoggerBuffer;
typedef std::lock_guard<std::mutex> locker;

// ---- C++14/17 specific stuff ----
#if _MSVC_LANG >= 201703L || __cplusplus >= 201703L
    #define BLOGGER_PROCESS_PACK(formatter, args) (formatter.handle_pack(std::forward<Args>(args)), ...)
    #include <string_view>
    typedef std::basic_string_view<bl_char, std::char_traits<bl_char>> BLoggerInString;
#elif _MSVC_LANG >= 201402L || __cplusplus >= 201402L
    #define BLOGGER_PROCESS_PACK(formatter, args) int expander[] = { 0, ( (void) formatter.handle_pack(std::forward<Args>(args)), 0) ... }
    typedef const std::basic_string<bl_char, std::char_traits<bl_char>>& BLoggerInString;
#else
    #error "BLogger requires at least /std:c++14"
#endif

// ---- Some useful defines ----
#define BLOGGER_BUFFER_SIZE 128
#define BLOGGER_TIMESTAMP BLOGGER_MAKE_UNICODE("%H:%M:%S") // should be made customizable later
#define BLOGGER_ARG_PATTERN BLOGGER_MAKE_UNICODE("{}")
