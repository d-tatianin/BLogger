#pragma once

#include <stdio.h>
#include <time.h>

#include "blogger/Core.h"

#ifdef _WIN32
    #define BLOGGER_UPDATE_TIME(to, from) localtime_s(&to, &from)
    #ifdef BLOGGER_UNICODE_MODE
        #define BLOGGER_OPEN_FILE(file, path) _wfopen_s(&file, path.c_str(), BLOGGER_FILEMODE)
    #else
        #define BLOGGER_OPEN_FILE(file, path) fopen_s(&file, path.c_str(), BLOGGER_FILEMODE)
    #endif

    namespace bl {
    
        // A windows-only safer version of alloca that
        // can allocate on the heap as well. Requires freeing.
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
    }
    
    #define BLOGGER_STACK_ALLOC(size, out_ptr) ::bl::anonymous_stack_allocator anonbuf_##out_ptr((size) * sizeof(char_t)); \
                                       out_ptr = static_cast<decltype(out_ptr)>(anonbuf_##out_ptr.buffer)

    #ifdef BLOGGER_UNICODE_MODE
        #include <io.h>
        #include <fcntl.h>
        #include <stringapiset.h>

        namespace bl {
            static inline void init_unicode()
            {
                auto ignored = _setmode(_fileno(stdout), _O_U16TEXT);
                ignored =      _setmode(_fileno(stdin),  _O_U16TEXT);
                ignored =      _setmode(_fileno(stderr), _O_U16TEXT);
            }
        }
        #define BLOGGER_INIT_UNICODE_MODE ::bl::init_unicode
    #else
        static inline void blogger_dummy_func() {}
        #define BLOGGER_INIT_UNICODE_MODE blogger_dummy_func
    #endif
#elif defined(__linux__)
    #include <cstring>
    #include <algorithm>
    #include <clocale>

    #define BLOGGER_UPDATE_TIME(to, from) localtime_r(&from, &to)
    #define BLOGGER_STACK_ALLOC(size, out_ptr) out_ptr = static_cast<decltype(out_ptr)>(alloca((size) * sizeof(char_t)))

    #ifdef BLOGGER_UNICODE_MODE
        namespace bl {

            inline void open_unicode_file(FILE*& out_file, const char_t* path)
            {
                size_t file_size = BLOGGER_STRING_LENGTH(path) * 2;
                char* narrow; BLOGGER_STACK_ALLOC(file_size, narrow);

                auto size = wcstombs(narrow, path, file_size);
                if (size == -1)
                {
                    out_file = nullptr;
                    return;
                }

                narrow[size] = '\0';

                auto fptr = fopen(narrow, BLOGGER_FILEMODE);
                out_file = fptr;
            }
        }
        #define BLOGGER_OPEN_FILE(file, path) ::bl::open_unicode_file(file, path.c_str())
    #else
        #define BLOGGER_OPEN_FILE(file, path) file = fopen(path.c_str(), BLOGGER_FILEMODE)
    #endif

    #ifdef BLOGGER_UNICODE_MODE
        namespace bl {
            inline void init_unicode()
            {
                setlocale(LC_ALL, "en_US.utf8");
            }
        }
        #define BLOGGER_INIT_UNICODE_MODE init_unicode
    #else
        inline void blogger_dummy_func() {}
        #define BLOGGER_INIT_UNICODE_MODE blogger_dummy_func
    #endif
#else
    #error Sorry, your platform is currently not supported! Please let me know it you think it should be.
#endif

#define BLOGGER_FILE_WRITE(data, size, file) fwrite(data, sizeof(char), size, file)
