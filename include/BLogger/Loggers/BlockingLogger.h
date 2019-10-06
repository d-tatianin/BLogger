#pragma once

#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/LogLevels.h"
#include "BLogger/Sinks/BaseSink.h"

namespace BLogger {

    class BlockingLogger : public BaseLogger
    {
    public:
        BlockingLogger(
            BLoggerInString tag,
            level lvl,
            bool default_pattern = true
        ) : BaseLogger(
            tag,
            lvl,
            default_pattern
        )
        {
        }

        void Flush() override
        {
            for (auto& sink : *m_Sinks)
            {
                sink->flush();
            }
        }
    private:
        void Post(BLoggerLogMessage&& msg) override
        {
            msg.finalize_format();

            for (auto& sink : *m_Sinks)
            {
                sink->write(msg);
            }
        }
    };
}
