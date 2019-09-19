#pragma once

#include <utility>

// MSVC still sets __cplusplus to 199711L
// this bug has been known for years
// but is still not fixed.
// So we use this workaround:
#if _MSVC_LANG >= 201703L || __cplusplus >= 201703L
    #define BLOGGER_PROCESS_PACK(formatter, args) (formatter.handle_pack(std::forward<Args>(args)), ...)
#elif _MSVC_LANG >= 201402L || __cplusplus >= 201402L
    #define BLOGGER_PROCESS_PACK(formatter, args) int expander[] = { 0, ( (void) formatter.handle_pack(std::forward<Args>(args)), 0) ... }
#else
    #error "BLogger requires at least /std:c++14"
#endif

#define BLOGGER_FORMAT_TIMESTAMP(message, pattern) (message.data() [strftime(message.data() + 1,  message.size() - 1, pattern, message.time_point_ptr()) + 1] = ']')
