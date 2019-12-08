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
    #define BLOGGER_WIDEN_IF_NEEDED(str) L##str
    #define BLOGGER_COUT ::std::wcout
    #ifdef _WIN32
        #define BLOGGER_FILEMODE BLOGGER_WIDEN_IF_NEEDED("w")
    #elif defined(__linux__)
        #define BLOGGER_EXTRA_BYTES 0
        #define BLOGGER_FILEMODE "w"
    #endif
    #define BLOGGER_STRING_LENGTH(string) wcslen(string)
    #define BLOGGER_TIME_TO_STRING(out, out_size, in_format, in_time) wcsftime(out, out_size, in_format, in_time)
    #define BLOGGER_FORMAT_STRING(out, out_size, in_format, ...) swprintf(out, out_size, in_format, __VA_ARGS__)
    #define BLOGGER_OSTREAM ::std::wostream
    #define BLOGGER_STD_TO_STRING ::std::to_wstring
#else
    typedef char bl_char;
    #define BLOGGER_WIDEN_IF_NEEDED(str) str
    #define BLOGGER_COUT ::std::cout
    #define BLOGGER_FILEMODE "w"
    #define BLOGGER_STRING_LENGTH(string) strlen(string)
    #define BLOGGER_TIME_TO_STRING(out, out_size, in_format, in_time) strftime(out, out_size, in_format, in_time)
    #define BLOGGER_FORMAT_STRING(out, out_size, in_format, ...) snprintf(out, out_size, in_format, __VA_ARGS__)
    #define BLOGGER_OSTREAM ::std::ostream
    #define BLOGGER_STD_TO_STRING ::std::to_string
#endif

#ifdef _WIN32 // CLRF?
    #define BLOGGER_TRUE_SIZE(size) static_cast<size_t>(size) + 3
#elif defined(__linux__)
    #define BLOGGER_TRUE_SIZE(size) size
#endif

typedef std::basic_string<bl_char, std::char_traits<bl_char>> BLoggerString;
typedef std::basic_stringstream<bl_char, std::char_traits<bl_char>> BLoggerStringStream;
typedef std::vector<bl_char> bl_string;
typedef bl_string BLoggerBuffer;
typedef std::lock_guard<std::mutex> locker;

// ---- C++14/17 specific stuff ----
#if _MSVC_LANG >= 201703L || __cplusplus >= 201703L
    #define BLOGGER_FOR_EACH_DO(what, args, ...) (what(__VA_ARGS__, std::forward<Args>(args)), ...)
    #include <string_view>
    typedef std::basic_string_view<bl_char, std::char_traits<bl_char>> BLoggerInString;
#elif _MSVC_LANG >= 201402L || __cplusplus >= 201402L
    #define BLOGGER_FOR_EACH_DO(what, args, ...) int expander[] = { 0, ( what(__VA_ARGS__, std::forward<Args>(args)), 0) ... }
    typedef const std::basic_string<bl_char, std::char_traits<bl_char>>& BLoggerInString;
#else
    #error "BLogger requires at least /std:c++14"
#endif

// ---- SFINAE (Only enable the 'log' functions if all arguments are ostream-compatible) ----
template<typename T>
using try_insert = decltype(std::declval<BLOGGER_OSTREAM&>() << std::declval<T>());

template<typename T>
struct is_ostream_ref : public std::false_type
{
};

template<>
struct is_ostream_ref<BLOGGER_OSTREAM&> : public std::true_type
{
};

template<typename T>
struct is_ostream_insertable
{
    static constexpr bool value = is_ostream_ref<try_insert<T>>::value;
};

template<class...> struct are_all_true : std::true_type { };
template<class Arg1> struct are_all_true<Arg1> : Arg1 { };
template<class Arg1, class... Argn>
struct are_all_true<Arg1, Argn...>
    : std::conditional_t<Arg1::value, are_all_true<Argn...>, Arg1> {};

template<typename... Args>
using enable_if_ostream_insertable = std::enable_if<are_all_true<is_ostream_insertable<Args>...>::value, void>;

template<typename... Args>
using enable_if_ostream_insertable_t = typename enable_if_ostream_insertable<Args...>::type;

template<typename T>
using enable_if_integral = std::enable_if<std::is_integral<T>::value, BLoggerString>;

template<typename T>
using enable_if_integral_t = typename enable_if_integral<T>::type;

template<typename T>
using enable_if_not_integral_and_not_string =
std::enable_if<
    !std::is_same<typename std::decay<T>::type, BLoggerString>::value &&
    !std::is_integral<typename std::decay<T>::type>::value &&
    is_ostream_insertable<T>::value
    , BLoggerString>;

template<typename T>
using enable_if_not_integral_and_not_string_t = typename enable_if_not_integral_and_not_string<T>::type;

template<typename T>
using enable_if_string = std::enable_if<std::is_same<typename std::decay<T>::type, BLoggerString>::value, T&&>;

template<typename T>
using enable_if_string_t = typename enable_if_string<T>::type;

template<typename T>
enable_if_integral_t<T> to_string(T arg)
{
    return BLOGGER_STD_TO_STRING(arg);
}

template<typename T>
enable_if_not_integral_and_not_string_t<T> to_string(T&& arg)
{
    BLoggerStringStream ss;
    ss << std::forward<T>(arg);
    return ss.str();
}

template<typename T>
enable_if_string_t<T> to_string(T&& str)
{
    return std::forward<T>(str);
}

#define BLOGGER_TO_STRING(what) to_string(what)
#define BLOGGER_INFINITE 0u
