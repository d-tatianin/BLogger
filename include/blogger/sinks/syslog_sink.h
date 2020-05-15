#pragma once

#ifdef __linux__
    #include <syslog.h>
    #include <cstdlib>
#else
    #define syslog(a, b, c)  (void) a; (void) b; (void) c
    #define openlog(a, b, c) (void) a; (void) b; (void) c
    #define LOG_PID  0
    #define LOG_USER 0
    #define LOG_INFO 0
#endif

#include "sink.h"
#include "blogger/loggers/logger.h"

namespace bl {
    class syslog_sink : public sink
    {
    public:
        syslog_sink()
        {
            openlog(current_tag().data(), LOG_PID, LOG_USER);
        }

        void write(log_message& msg) override
        {
          #if defined(BLOGGER_UNICODE_MODE) && defined (__linux__)
            constexpr size_t syslog_message_size = 1024;
            char message[syslog_message_size];

            auto size = wcstombs(message, msg.data(), syslog_message_size - 1);
            if (size == -1)
                return;

            message[size] = '\0';
          #else
            auto* message = msg.data();
          #endif
            syslog(
                LOG_INFO,
                "%s",
                message
            );
        }

        void flush() override
        {
        }

        void set_tag(in_string tag) override
        {
          #if defined(BLOGGER_UNICODE_MODE) && defined (__linux__)
            constexpr size_t syslog_tag_size = 256;
            char narrow_tag[syslog_tag_size];

            auto size = wcstombs(narrow_tag, tag.data(), syslog_tag_size - 1);
            if (size == -1)
                return;

            narrow_tag[size] = '\0';

            current_tag() = narrow_tag;
          #else
            current_tag() = tag;
          #endif
            openlog(current_tag().data(), LOG_PID, LOG_USER);
        }

        static std::string& current_tag()
        {
            static std::string tag = "Unnamed";
            return tag;
        }
    };
}

#ifndef __linux__
    #undef syslog
    #undef openlog
    #undef LOG_PID
    #undef LOG_USER
    #undef LOG_INFO
#endif
