#pragma once

#include <utility>

// ---- Custom BLogger types ----
typedef char bl_char;
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
#define BLOGGER_TIMESTAMP "%H:%M:%S" // should be made customizable later
#define BLOGGER_ARG_PATTERN "{}"
