#pragma once

#include <iostream>

#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/Formatter/FormatUtilities.h"
#include "BLogger/Sinks/BaseSink.h"

namespace BLogger {

    class StdoutSink : public BaseSink
    {
    protected:
        static std::mutex s_GlobalWrite;
    public:
        static std::mutex& GetGlobalWriteLock()
        {
            return s_GlobalWrite;
        }

        void write(BLoggerLogMessage& msg) override
        {
            locker lock(s_GlobalWrite);

            std::cout.write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            locker lock(s_GlobalWrite);
            std::cout.flush();
        }
    };
}

std::mutex BLogger::StdoutSink::s_GlobalWrite;
