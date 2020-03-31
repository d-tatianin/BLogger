#pragma once

#include <iostream>

#include "Sink.h"
#include "BLogger/Loggers/Logger.h"

namespace bl {

    class StdoutSink : public Sink
    {
    public:
        StdoutSink()
        {
        }

        static std::mutex& GetGlobalWriteLock()
        {
            static std::mutex globalWrite;

            return globalWrite;
        }

        void write(BLoggerLogMessage& msg) override
        {
            auto& wl = GetGlobalWriteLock();
            locker lock(wl);

            BLOGGER_COUT.write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = GetGlobalWriteLock();
            locker lock(wl);

            BLOGGER_COUT.flush();
        }
    };
}
