#pragma once

#include "BLogger/Loggers/LogMessage.h"

namespace BLogger {

    class BaseSink
    {
    public:
        virtual void write(BLoggerLogMessage& msg) = 0;
        virtual void flush() = 0;

        // Is there a better way to forward the tag
        // to the file sink?
        virtual void set_tag(BLoggerInString tag) {}

        virtual ~BaseSink() {}
    };
}
