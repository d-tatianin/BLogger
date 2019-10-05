#pragma once

#include <stdio.h>
#include <time.h>

#ifdef _WIN32
    #define UPDATE_TIME(to, from) localtime_s(&to, &from)
    #define OPEN_FILE(file, path) fopen_s(&file, path.c_str(), "w")
    #define MEMORY_COPY(dst, dst_size, src, src_size) memcpy_s(dst, dst_size, src, src_size)
    #define MEMORY_MOVE(dst, dst_size, src, src_size) memmove_s(dst, dst_size, src, src_size)

    // A windows-only safer version of alloca that
    // might allocate on the heap as well. Requires freeing.
    struct anonymous_stack_allocator
    {
    public:
        void* buffer;
    
        anonymous_stack_allocator(size_t size)
            : buffer(nullptr)
        {
            buffer = _malloca(size);
        }
    
        ~anonymous_stack_allocator()
        {
            _freea(buffer);
        }
    };
    
    #define STACK_ALLOC(size, out_ptr) anonymous_stack_allocator anonbuf_##out_ptr(size); \
                                       out_ptr = static_cast<decltype(out_ptr)>(anonbuf_##out_ptr.buffer)
#else
    #include <cstring>
    #include <algorithm>
    #define UPDATE_TIME(to, from) localtime_r(&from, &to)
    #define OPEN_FILE(file, path) file = fopen(path.c_str(), "w")
    #define MEMORY_COPY(dst, dst_size, src, src_size) memcpy(dst, src, src_size)
    #define MEMORY_MOVE(dst, dst_size, src, src_size) memmove(dst, src, src_size)
    #define STACK_ALLOC(size, out_ptr) out_ptr = static_cast<decltype(out_ptr)>(alloca(size))
#endif
