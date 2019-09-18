#pragma once

#include <utility>

// MSVC still sets __cplusplus to 199711L
// this bug has been known for years
// but is still not fixed.
// So we use this workaround:
#if _MSVC_LANG >= 201703L || __cplusplus >= 201703L
    #define BLOGGER_PACK_BEGIN(formatter, args) (formatter.handle_pack(std::forward<Args>(args)), ...)
#elif _MSVC_LANG >= 201402L || __cplusplus >= 201402L
    #define BLOGGER_PACK_BEGIN(formatter, args) (int expander[] = { 0, ( (void) formatter.handle_pack(std::forward<Args>(args)), 0) ... })
#else
    #error("-std has to be at least c++14")
#endif

#define BLOGGER_PACK_END(formatter, args) (formatter.finalize_pack(args...)) 

#define BLOGGER_FORMAT_TIMESTAMP(message, pattern) (message.data() [strftime(message.data() + 1,  message.size() - 1, pattern, message.time_point_ptr()) + 1] = ']')
