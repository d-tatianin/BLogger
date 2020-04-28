#pragma once

#include <iostream>

#include "Sink.h"
#include "blogger/loggers/logger.h"

namespace bl {
    template<BLOGGER_OSTREAM& stream>
    class console_sink : public sink
    {
    public:
        console_sink()
        {
        }

        void write(log_message& msg) override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);

            stream.write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);

            stream.flush();
        }
    };

    // should have a clog sink as well?
    using stderr_sink = console_sink<BLOGGER_CERR>;
    using stdout_sink = console_sink<BLOGGER_COUT>;
}
