#pragma once

#include "blogger/loggers/logger.h"
#include "blogger/log_levels.h"
#include "blogger/sinks/sink.h"

namespace bl {

    class BlockingLogger : public logger
    {
    public:
        BlockingLogger(
            in_string tag,
            level lvl,
            bool default_pattern = true
        ) : logger(
            tag,
            lvl,
            default_pattern
        )
        {
        }

        void flush() override
        {
            for (auto& sink : *m_Sinks)
            {
                sink->flush();
            }
        }
    private:
        void post(log_message&& msg) override
        {
            msg.finalize_format();

            for (auto& sink : *m_Sinks)
            {
                sink->write(msg);
            }
        }
    };
}
