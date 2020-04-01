#pragma once

#pragma once

#include <iostream>

#include "blogger/Loggers/Logger.h"
#include "blogger/Core.h"
#include "blogger/Sinks/Sink.h"
#include "blogger/Sinks/StdoutSink.h"

namespace bl {

    class ColoredStdoutSink : public StdoutSink
    {
        void write(BLoggerLogMessage& msg) override
        {
            auto& wl = GetGlobalWriteLock();
            locker lock(wl);

            ScopedStdoutColor message_color(
                level_to_color(msg.log_level())
            );

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
