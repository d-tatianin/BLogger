#pragma once

#pragma once

#include <iostream>

#include "blogger/Loggers/Logger.h"
#include "blogger/Core.h"
#include "blogger/Sinks/Sink.h"
#include "blogger/Sinks/ConsoleSink.h"

namespace bl {

    template<BLOGGER_OSTREAM& stream>
    class ColoredConsoleSink : public ConsoleSink<stream>
    {
    public:
        void write(LogMessage& msg) override
        {
            auto& wl = GlobalConsoleWriteLock();
            locker lock(wl);

            ScopedConsoleColor<stream> message_color(
                msg.log_level().to_color()
            );

            stream.write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = GlobalConsoleWriteLock();
            locker lock(wl);
            stream.flush();
        }
    };

    using ColoredStderrSink = ColoredConsoleSink<BLOGGER_CERR>;
    using ColoredStdoutSink = ColoredConsoleSink<BLOGGER_COUT>;
}
