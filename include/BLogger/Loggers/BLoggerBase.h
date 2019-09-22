#pragma once

#include <ctime>

#include "BLogger/LogLevels.h"
#include "BLogger/Formatter/Formatter.h"
#include "BLogger/OS/Functions.h"

#define BLOGGER_INFINITE 0u

namespace BLogger {

    struct LogMsg
    {
    private:
        BLoggerFormatter fmt;
        BLoggerPattern ptrn;
        std::tm time_point;
        level lvl;
        bool log_to_stdout;
        bool log_to_file;
        bool colored;
        uint16_t sender_id;
    public:
        LogMsg(
            BLoggerFormatter&& fmt,
            BLoggerPattern ptrn,
            std::tm tp,
            level lvl,
            bool log_stdout,
            bool log_file,
            bool colored,
            uint16_t sender_id
        )
            : fmt(std::move(fmt)),
            ptrn(ptrn),
            time_point(tp),
            lvl(lvl),
            log_to_stdout(log_stdout),
            log_to_file(log_file),
            colored(colored),
            sender_id(sender_id)
        {
        }

        void finalize_format()
        {
            fmt.merge_pattern(ptrn, time_point_ptr(), lvl);
        }

        charT* data()
        {
            return fmt.data();
        }

        size_t size()
        {
            return fmt.size();
        }

        level log_level()
        {
            return lvl;
        }

        bool color()
        {
            return colored;
        }

        bool console()
        {
            return log_to_stdout;
        }

        bool file()
        {
            return log_to_file;
        }

        uint16_t sender()
        {
            return sender_id;
        }
    private:
        std::tm* time_point_ptr()
        {
            return &time_point;
        }
    };

    class BLoggerBase
    {
    private:
        static uint16_t unique_id;
    protected:
        const uint16_t    m_ID;
        std::string       m_Tag;
        std::string       m_CachedPattern;
        BLoggerPattern    m_Pattern;
        level             m_Filter;
        bool              m_LogToConsole;
        bool              m_LogToFile;
        bool              m_ColoredOutput;
    public:
        BLoggerBase()
            : m_ID(unique_id++),
            m_Tag("Unnamed"),
            m_CachedPattern("[{ts}][{lvl}][{tag}] {msg}"),
            m_Pattern(),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            SetPattern(m_CachedPattern);
        }

        BLoggerBase(const std::string& tag)
            : m_ID(unique_id++),
            m_Tag(tag),
            m_CachedPattern("[{ts}][{lvl}][{tag}] {msg}"),
            m_Pattern(),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            SetPattern(m_CachedPattern);
        }

        BLoggerBase(
            const std::string& tag, 
            level lvl,
            bool default_pattern
        )
            : m_ID(unique_id++),
            m_Tag(tag),
            m_CachedPattern("[{ts}][{lvl}][{tag}] {msg}"),
            m_Pattern(),
            m_Filter(lvl),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false)
        {
            if (default_pattern)
            {
                SetPattern(m_CachedPattern);
            }
        }

        BLoggerBase(const BLoggerBase& other) = delete;
        BLoggerBase& operator=(const BLoggerBase& other) = delete;

        BLoggerBase(BLoggerBase&& other) = default;
        BLoggerBase& operator=(BLoggerBase&& other) = default;

        void SetPattern(const std::string& pattern)
        {
            m_CachedPattern = pattern;
            m_Pattern.init();
            m_Pattern.set_pattern(pattern, m_Tag);
        }

        virtual bool InitFileLogger(
            const std::string& directoryPath,
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

        template <typename T = std::string>
        void Log(level lvl, const T& message)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                message, 
                std::strlen(message)
            );

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                std::move(formatter),
                m_Pattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
            });
        }

        template<typename... Args>
        void Log(level lvl, const std::string& formattedMsg, Args&& ... args)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                formattedMsg.c_str(), 
                formattedMsg.size()
            );

            BLOGGER_PROCESS_PACK(formatter, args);

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                std::move(formatter),
                m_Pattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile,
                m_ColoredOutput,
                m_ID
            });
        }

        template <typename T>
        void Trace(const T& message)
        {
            Log(level::trace, message);
        }

        template <typename T>
        void Debug(const T& message)
        {
            Log(level::debug, message);
        }

        template <typename T>
        void Info(const T& message)
        {
            Log(level::info, message);
        }

        template <typename T>
        void Warning(const T& message)
        {
            Log(level::warn, message);
        }

        template <typename T>
        void Error(const T& message)
        {
            Log(level::error, message);
        }

        template <typename T>
        void Critical(const T& message)
        {
            Log(level::crit, message);
        }

        template<typename T, typename... Args>
        void Trace(const T& formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Debug(const T& formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Info(const T& formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Warning(const T& formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Error(const T& formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Critical(const T& formattedMsg, Args&& ... args)
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

        virtual void Post(LogMsg&& msg) = 0;
    };

    uint16_t BLoggerBase::unique_id = 1;
}
