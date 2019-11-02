#pragma once

#include <iostream>

#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/Formatter/FormatUtilities.h"
#include "BLogger/Sinks/BaseSink.h"

namespace BLogger {

    class StdoutSink : public BaseSink
    {
    public:
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

            std::cout.write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = GetGlobalWriteLock();
            locker lock(wl);

            std::cout.flush();
        }
    };
}
