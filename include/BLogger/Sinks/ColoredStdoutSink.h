#pragma once

#pragma once

#include <iostream>

#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/Formatter/FormatUtilities.h"
#include "BLogger/Sinks/BaseSink.h"
#include "BLogger/Sinks/StdoutSink.h"

namespace BLogger {

    class ColoredStdoutSink : public StdoutSink
    {
        void write(BLoggerLogMessage& msg) override
        {
            locker lock(s_GlobalWrite);

            switch (msg.log_level())
            {
                case level::trace: set_output_color(BLOGGER_TRACE_COLOR); break;
                case level::debug: set_output_color(BLOGGER_DEBUG_COLOR); break;
                case level::info:  set_output_color(BLOGGER_INFO_COLOR);  break;
                case level::warn:  set_output_color(BLOGGER_WARN_COLOR);  break;
                case level::error: set_output_color(BLOGGER_ERROR_COLOR); break;
                case level::crit:  set_output_color(BLOGGER_CRIT_COLOR);  break;
            }

            std::cout.write(
                msg.data(),
                msg.size()
            );

            set_output_color(BLOGGER_RESET);
        }

        void flush() override
        {
            locker lock(s_GlobalWrite);
            std::cout.flush();
        }
    };
}
