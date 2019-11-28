#pragma once

#include <iostream>

#include "BaseSink.h"
#include "BLogger/Loggers/BaseLogger.h"

namespace BLogger {

    class StdoutSink : public BaseSink
    {
    public:
        StdoutSink()
        {
        }

        static std::mutex& GetGlobalWriteLock()
        {
            // A little workaround to extend
            // the lifetime of this object
            static std::mutex* globalWrite 
                = new std::mutex();

            return *globalWrite;
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
