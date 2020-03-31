#pragma once

#include "BLogger/Loggers/LogMessage.h"

namespace bl {

    class Sink
    {
    public:
        virtual void write(BLoggerLogMessage& msg) = 0;
        virtual void flush() = 0;

        virtual ~Sink() {}
    };
}
