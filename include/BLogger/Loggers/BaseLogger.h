#pragma once

#include <ctime>

#include "BLogger/LogLevels.h"
#include "BLogger/Formatter/Formatter.h"
#include "BLogger/OS/Functions.h"
#include "BLogger/Loggers/LogMessage.h"

#define BLOGGER_INFINITE 0u
#define BLOGGER_DEFAULT_PATTERN "[{ts}][{lvl}][{tag}] {msg}"

namespace BLogger {

    class BLoggerBase
    {
    private:
        static uint16_t unique_id;
    protected:
        const uint16_t       m_ID;
        BLoggerInString      m_Tag;
        BLoggerInString      m_CachedPattern;
        BLoggerSharedPattern m_CurrentPattern;
        level                m_Filter;
        bool                 m_LogToConsole;
        bool                 m_LogToFile;
        bool                 m_ColoredOutput;
    public:
        BLoggerBase()
            : m_ID(unique_id++),
            m_Tag("Unnamed"),
            m_CachedPattern(BLOGGER_DEFAULT_PATTERN),
            m_CurrentPattern(new BLoggerPattern(m_ID)),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            m_CurrentPattern->init();
            m_CurrentPattern->set_pattern(BLOGGER_DEFAULT_PATTERN, m_Tag);
        }

        BLoggerBase(const BLoggerInString& tag)
            : m_ID(unique_id++),
            m_Tag(tag),
            m_CachedPattern(BLOGGER_DEFAULT_PATTERN),
            m_CurrentPattern(new BLoggerPattern(m_ID)),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            m_CurrentPattern->init();
            m_CurrentPattern->set_pattern(BLOGGER_DEFAULT_PATTERN, m_Tag);
        }

        BLoggerBase(
            const BLoggerInString& tag,
            level lvl,
            bool default_pattern
        )
            : m_ID(unique_id++),
            m_Tag(tag),
            m_CachedPattern(BLOGGER_DEFAULT_PATTERN),
            m_CurrentPattern(new BLoggerPattern(m_ID)),
            m_Filter(lvl),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            if (default_pattern)
            {
                m_CurrentPattern->init();
                m_CurrentPattern->set_pattern(BLOGGER_DEFAULT_PATTERN, m_Tag);
            }
        }

        BLoggerBase(const BLoggerBase& other) = delete;
        BLoggerBase& operator=(const BLoggerBase& other) = delete;

        BLoggerBase(BLoggerBase&& other) = default;
        BLoggerBase& operator=(BLoggerBase&& other) = default;

        void SetPattern(const BLoggerInString& pattern)
        {
            m_CachedPattern = pattern;
            BLoggerPattern* newPattern = new BLoggerPattern(m_ID);
            newPattern->init();
            newPattern->set_pattern(pattern, m_Tag);

            m_CurrentPattern.reset(newPattern);
        }

        virtual bool InitFileLogger(
            const BLoggerInString& directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true) = 0;

        virtual bool EnableFileLogger() = 0;
        virtual void TerminateFileLogger() = 0;

        void DisableFileLogger()
        {
            m_LogToFile = false;
        }

        void EnableConsoleLogger()
        {
            m_LogToConsole = true;
        }

        void DisableConsoleLogger()
        {
            m_LogToConsole = false;
        }

        void EnableColoredOutput()
        {
            m_ColoredOutput = true;
        }

        void DisableColoredOutput()
        {
            m_ColoredOutput = false;
        }

        virtual void Flush() = 0;

        void Log(level lvl, const BLoggerInString& message)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                message.data(),
                message.size()
            );

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(),
                m_CurrentPattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
            });
        }

        void Log(level lvl, const bl_char* message)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                message,
                strlen(message)
            );

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(),
                m_CurrentPattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
                });
        }

        template<typename... Args>
        void Log(level lvl, const BLoggerInString& formattedMsg, Args&& ... args)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                formattedMsg.data(), 
                formattedMsg.size()
            );

            BLOGGER_PROCESS_PACK(formatter, args);

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(),
                m_CurrentPattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
            });
        }

        template<typename... Args>
        void Log(level lvl, const bl_char* formattedMsg, Args&& ... args)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                formattedMsg,
                strlen(formattedMsg)
            );

            BLOGGER_PROCESS_PACK(formatter, args);

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(),
                m_CurrentPattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
                });
        }

        void Trace(const BLoggerInString& message)
        {
            Log(level::trace, message);
        }

        void Debug(const BLoggerInString& message)
        {
            Log(level::debug, message);
        }

        void Info(const BLoggerInString& message)
        {
            Log(level::info, message);
        }

        void Warning(const BLoggerInString& message)
        {
            Log(level::warn, message);
        }

        void Error(const BLoggerInString& message)
        {
            Log(level::error, message);
        }

        void Critical(const BLoggerInString& message)
        {
            Log(level::crit, message);
        }

        void Trace(const bl_char* message)
        {
            Log(level::trace, message);
        }

        void Debug(const bl_char* message)
        {
            Log(level::debug, message);
        }

        void Info(const bl_char* message)
        {
            Log(level::info, message);
        }

        void Warning(const bl_char* message)
        {
            Log(level::warn, message);
        }

        void Error(const bl_char* message)
        {
            Log(level::error, message);
        }

        void Critical(const bl_char* message)
        {
            Log(level::crit, message);
        }

        template<typename... Args>
        void Trace(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Debug(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Info(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Warning(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Error(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Critical(const BLoggerInString& formattedMsg, Args&& ... args)
        {
            Log(level::crit, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Trace(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Debug(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Info(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Warning(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Error(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Critical(const bl_char* formattedMsg, Args&& ... args)
        {
            Log(level::crit, formattedMsg, std::forward<Args>(args)...);
        }

        void SetFilter(level lvl)
        {
            m_Filter = lvl;
        }

        virtual void SetTag(const std::string& tag) = 0;

        virtual ~BLoggerBase() {}
    protected:
        bool ShouldLog(level lvl)
        {
            if (m_Filter > lvl)
                return false;

            if (!m_LogToConsole && !m_LogToFile)
                return false;

            return true;
        }

        virtual void Post(BLoggerLogMessage&& msg) = 0;
    };

    uint16_t BLoggerBase::unique_id = 1;
}
