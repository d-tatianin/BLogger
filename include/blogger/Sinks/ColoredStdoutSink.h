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

            switch (msg.log_level())
            {
                case level::trace: set_output_color(BLOGGER_TRACE_COLOR); break;
                case level::debug: set_output_color(BLOGGER_DEBUG_COLOR); break;
                case level::info:  set_output_color(BLOGGER_INFO_COLOR);  break;
                case level::warn:  set_output_color(BLOGGER_WARN_COLOR);  break;
                case level::error: set_output_color(BLOGGER_ERROR_COLOR); break;
                case level::crit:  set_output_color(BLOGGER_CRIT_COLOR);  break;
            }

            BLOGGER_COUT.write(
                msg.data(),
                msg.size()
            );

            set_output_color(BLOGGER_RESET);
        }

        void flush() override
        {
            auto& wl = GetGlobalWriteLock();
            locker lock(wl);
            BLOGGER_COUT.flush();
        }
    };
}
