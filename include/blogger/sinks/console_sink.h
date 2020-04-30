#pragma once

#include <iostream>

#include "sink.h"
#include "blogger/loggers/logger.h"

namespace bl {
    template<ostream& stream>
    class console_sink : public sink
    {
    private:
        ostream& m_Stream = stream;
    public:
        console_sink()
        {
        }

        void write(log_message& msg) override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);

            underlying_stream().write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);

            underlying_stream().flush();
        }

        ostream& underlying_stream()
        {
            return m_Stream;
        }

        ostream& operator<<(in_string message)
        {
            return underlying_stream() << message;
        }
    };

    using stderr_sink = console_sink<BLOGGER_CERR>;
    using stdout_sink = console_sink<BLOGGER_COUT>;
    using stdlog_sink = console_sink<BLOGGER_CLOG>;
}
