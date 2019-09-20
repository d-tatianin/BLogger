#pragma once

#include "BLogger/Loggers/BLoggerBase.h"

namespace BLogger {

    class BLoggerBlock : public BLoggerBase
    {
    public:
        BLoggerBlock()
            : BLoggerBase()
        {
        }

        BLoggerBlock(const std::string& tag)
            : BLoggerBase(tag)
        {
        }

        BLoggerBlock(const std::string& tag, level lvl)
            : BLoggerBase(tag, lvl)
        {
        }

        void Flush() override
        {
            std::cout.flush();

            if (m_File)
                fflush(m_File);
        }

        ~BLoggerBlock()
        {

        }

    private:
        void Post(LogMsg&& msg) override
        {
            msg.finalize_format();

            if (m_LogToConsole)
            {
                if (m_ColoredOutput)
                {
                    switch (msg.log_level())
                    {
                    case level::trace: set_output_color(BLOGGER_TRACE_COLOR); break;
                    case level::debug: set_output_color(BLOGGER_DEBUG_COLOR); break;
                    case level::info:  set_output_color(BLOGGER_INFO_COLOR);  break;
                    case level::warn:  set_output_color(BLOGGER_WARN_COLOR);  break;
                    case level::error: set_output_color(BLOGGER_ERROR_COLOR); break;
                    case level::crit:  set_output_color(BLOGGER_CRIT_COLOR);  break;
                    }
                }

                std::cout.write(msg.data(), msg.size());

                if (m_ColoredOutput)
                    set_output_color(BLOGGER_RESET);
            }

            if (m_LogToFile)
            {
                if (m_BytesPerFile && msg.size() + 1 > m_BytesPerFile)
                    return;

                if (m_BytesPerFile && (m_CurrentBytes + msg.size() + 1) > m_BytesPerFile)
                {
                    if (m_CurrentLogFiles == m_MaxLogFiles)
                    {
                        if (!m_RotateLogs)
                            return;
                        else
                        {
                            m_CurrentLogFiles = 1;
                            m_CurrentBytes = 0;
                            NewLogFile();
                        }
                    }
                    else
                    {
                        m_CurrentBytes = 0;
                        ++m_CurrentLogFiles;
                        NewLogFile();
                    }

                }

                m_CurrentBytes += msg.size() + 1;

                fwrite(msg.data(), 1, msg.size(), m_File);
            }
        }
    };
}



#undef UPDATE_TIME
#undef OPEN_FILE
#undef BLOGGER_TS_PATTERN
#undef BLOGGER_TRACE_COLOR
#undef BLOGGER_DEBUG_COLOR
#undef BLOGGER_INFO_COLOR
#undef BLOGGER_WARN_COLOR
#undef BLOGGER_ERROR_COLOR
#undef BLOGGER_CRIT_COLOR
