#pragma once

#include <iostream>

#include "Sink.h"
#include "blogger/Loggers/Logger.h"

namespace bl {
    template<BLOGGER_OSTREAM& stream>
    class ConsoleSink : public Sink
    {
    public:
        ConsoleSink()
        {
        }

        void write(LogMessage& msg) override
        {
            auto& wl = GlobalConsoleWriteLock();
            locker lock(wl);

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

    // should have a clog sink as well?
    using StderrSink        = ConsoleSink<BLOGGER_CERR>;
    using StdoutSink        = ConsoleSink<BLOGGER_COUT>;
}
