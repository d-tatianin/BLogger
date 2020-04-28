#pragma once

#include <utility>
#include <iostream>
#include <vector>
#include <mutex>
#include <sstream>

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
    namespace bl {
        using char_t = wchar_t;
    }
    #define BLOGGER_WIDEN_IF_NEEDED(str) L##str
    #define BLOGGER_COUT ::std::wcout
    #define BLOGGER_CERR ::std::wcerr
    #ifdef _WIN32
        #define BLOGGER_FILEMODE BLOGGER_WIDEN_IF_NEEDED("w")
    #elif defined(__linux__)
        #define BLOGGER_FILEMODE "w"
    #endif
    #define BLOGGER_STRING_LENGTH(string) wcslen(string)
    #define BLOGGER_TIME_TO_STRING(out, out_size, in_format, in_time) wcsftime(out, out_size, in_format, in_time)
    #define BLOGGER_OSTREAM ::std::wostream
    #define BLOGGER_STD_TO_STRING ::std::to_wstring
#else
    namespace bl {
        using char_t = char;
    }
    #define BLOGGER_WIDEN_IF_NEEDED(str) str
    #define BLOGGER_COUT ::std::cout
    #define BLOGGER_CERR ::std::cerr
    #define BLOGGER_FILEMODE "w"
    #define BLOGGER_STRING_LENGTH(string) strlen(string)
    #define BLOGGER_TIME_TO_STRING(out, out_size, in_format, in_time) strftime(out, out_size, in_format, in_time)
    #define BLOGGER_OSTREAM ::std::ostream
    #define BLOGGER_STD_TO_STRING ::std::to_string
#endif

#ifdef _WIN32 // CRLF?
    #define BLOGGER_TRUE_SIZE(size) static_cast<size_t>(size) + 1
#elif defined(__linux__)
    #define BLOGGER_TRUE_SIZE(size) size
#endif

namespace bl {
    using string = std::basic_string<char_t, std::char_traits<char_t>>;
    using stringstream = std::basic_stringstream<char_t, std::char_traits<char_t>>;
    using locker = std::lock_guard<std::mutex>;
}
// ---- C++14/17 specific stuff ----
#if _MSVC_LANG >= 201703L || __cplusplus >= 201703L
    #define BLOGGER_FOR_EACH_DO(what, args, ...) (what(__VA_ARGS__, std::forward<Args>(args)), ...)
    #include <string_view>
    namespace bl {
        using in_string = std::basic_string_view<char_t, std::char_traits<char_t>>;
    }
#elif _MSVC_LANG >= 201402L || __cplusplus >= 201402L
    #define BLOGGER_FOR_EACH_DO(what, args, ...) int expander[] = { 0, ( what(__VA_ARGS__, std::forward<Args>(args)), 0) ... }
    namespace bl {
        using in_string = const std::basic_string<char_t, std::char_traits<char_t>>&;
    }
#else
    #error "BLogger requires at least /std:c++14"
#endif

namespace bl {
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
    using enable_if_arithmetic = std::enable_if<std::is_arithmetic<T>::value, string>;

    template<typename T>
    using enable_if_arithmetic_t = typename enable_if_arithmetic<T>::type;

    template<typename T>
    using enable_if_not_arithmetic_and_not_string =
        std::enable_if<
        !std::is_same<typename std::decay<T>::type, string>::value &&
        !std::is_arithmetic<typename std::decay<T>::type>::value &&
        is_ostream_insertable<T>::value
        , string>;

    template<typename T>
    using enable_if_not_arithmetic_and_not_string_t = typename enable_if_not_arithmetic_and_not_string<T>::type;

    template<typename T>
    using enable_if_string = std::enable_if<std::is_same<typename std::decay<T>::type, string>::value, T&&>;

    template<typename T>
    using enable_if_string_t = typename enable_if_string<T>::type;

    template<typename T>
    enable_if_arithmetic_t<T> to_string(T arg)
    {
        return BLOGGER_STD_TO_STRING(arg);
    }

    template<typename T>
    enable_if_not_arithmetic_and_not_string_t<T> to_string(T&& arg)
    {
        stringstream ss;
        ss << std::forward<T>(arg);
        return ss.str();
    }

    template<typename T>
    enable_if_string_t<T> to_string(T&& str)
    {
        return std::forward<T>(str);
    }

    template<>
    inline string to_string(char_t arg)
    {
        return string(1, arg);
    }

    constexpr size_t infinite = 0u;
}

#define BLOGGER_TO_STRING(what) ::bl::to_string(what)
