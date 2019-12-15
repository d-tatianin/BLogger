#pragma once

#include "BLogger/Loggers/LogMessage.h"

namespace BLogger {

    class BaseSink
    {
    public:
        virtual void write(BLoggerLogMessage& msg) = 0;
        virtual void flush() = 0;

        virtual ~BaseSink() {}
    };
}
