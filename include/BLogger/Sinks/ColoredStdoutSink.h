#pragma once

#pragma once

#include <iostream>

#include "BLogger/Loggers/Logger.h"
#include "BLogger/Core.h"
#include "BLogger/Sinks/Sink.h"
#include "BLogger/Sinks/StdoutSink.h"

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
