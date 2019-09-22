#pragma once

#include "BLogger/Loggers/BLoggerBase.h"
#include "BLogger/LogLevels.h"
#include "BLogger/Loggers/FileHelper.h"

namespace BLogger {

    class BLoggerBlock : public BLoggerBase
    {
    private:
        FileHelper m_File;
    public:
        BLoggerBlock()
            : BLoggerBase(),
            m_File()
        {
        }

        BLoggerBlock(const std::string& tag)
            : BLoggerBase(tag),
            m_File()
        {
        }

        BLoggerBlock(
            const std::string& tag,
            level lvl,
            bool default_pattern = true
        )
           : BLoggerBase(tag, lvl, default_pattern),
            m_File()
        {
        }

        void Flush() override
        {
            std::cout.flush();

            if (m_File)
                m_File.flush();
        }

        bool InitFileLogger(
            const std::string& directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true
        ) override
        {
            m_File.init(
                directoryPath,
                m_Tag,
                bytesPerFile,
                maxLogFiles,
                rotateLogs
            );

            return m_File.ok();
        }

        bool EnableFileLogger() override
        {
            if (!m_File)
            {
                Error("Could not enable the file logger. Did you call InitFileLogger?");
                return false;
            }

            m_LogToFile = true;
            return true;
        }

        void TerminateFileLogger() override
        {
            m_File.terminate();
        }

        void SetTag(const std::string& tag) override
        {
            m_Tag = tag;
            SetPattern(m_CachedPattern);
            m_File.setTag(tag);
        }

        ~BLoggerBlock()
        {
            m_File.terminate();
        }

    private:
        void Post(LogMsg&& msg) override
        {
            msg.finalize_format();

            if (msg.console())
            {
                if (msg.color())
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

                if (msg.color())
                    set_output_color(BLOGGER_RESET);
            }

            if (msg.file())
            {
                if (m_File.ready())
                    m_File.write(msg.data(), msg.size());
            }
        }
    };
}
