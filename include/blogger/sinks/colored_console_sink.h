#pragma once

#pragma once

#include <iostream>

#include "blogger/loggers/logger.h"
#include "blogger/core.h"
#include "blogger/sinks/sink.h"
#include "blogger/sinks/console_sink.h"

namespace bl {

    template<ostream& stream>
    class colored_console_sink : public console_sink<stream>
    {
    public:
        void write(log_message& msg) override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);

            scoped_console_color<stream> message_color(
                msg.log_level().to_color()
            );

            this->underlying_stream().write(
                msg.data(),
                msg.size()
            );
        }

        void flush() override
        {
            auto& wl = global_console_write_lock();
            locker lock(wl);
            this->underlying_stream().flush();
        }

        ostream& operator<<(color c)
        {
            return this->underlying_stream() << c;
        }
    };

    using colored_stderr_sink = colored_console_sink<BLOGGER_CERR>;
    using colored_stdout_sink = colored_console_sink<BLOGGER_COUT>;
    using colored_stdlog_sink = colored_console_sink<BLOGGER_CLOG>;
}
