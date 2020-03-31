#pragma once

#include <ctime>

#include "BLogger/Formatter.h"
#include "BLogger/Loggers/LogMessage.h"
#include "BLogger/OS/Functions.h"
#include "BLogger/Sinks/Sink.h"
#include "BLogger/Sinks/StdoutSink.h"
#include "BLogger/LogLevels.h"

#define BLOGGER_DEFAULT_PATTERN BLOGGER_WIDEN_IF_NEEDED("[{ts}][{lvl}][{tag}] {msg}")

namespace bl {

    typedef std::unique_ptr<Sink>
        SinkPtr;
    typedef std::vector<SinkPtr>
        Sinks;
    typedef std::shared_ptr<Sinks>
        SharedSinks;

    // ---- BLogger logger properties struct ----
    // Used for customizing the logger.
    struct Props
    {
        bool async;

        bool console_logger;
        bool colored;
        BLoggerString tag;
        BLoggerString pattern;
        level filter;

        bool file_logger;
        BLoggerString path;
        size_t bytes_per_file;
        size_t log_files;
        bool rotate_logs;

        Props()
            : async(true),
            console_logger(true),
            colored(true),
            tag(BLOGGER_WIDEN_IF_NEEDED("Unnamed")),
            pattern(BLOGGER_WIDEN_IF_NEEDED("")),
            filter(level::trace),
            file_logger(false),
            path(BLOGGER_WIDEN_IF_NEEDED("")),
            bytes_per_file(BLOGGER_INFINITE),
            log_files(0),
            rotate_logs(true)
        {
        }
    };

    class Logger
    {
    protected:
        BLoggerString         m_Tag;
        BLoggerString         m_CurrentPattern;
        BLoggerString         m_CachedPattern;
        SharedSinks           m_Sinks;
        level                 m_Filter;
    public:
        using Ptr = std::shared_ptr<Logger>;

        Logger(
            BLoggerInString tag,
            level lvl,
            bool default_pattern
        ) : m_Tag(tag),
            m_CachedPattern(BLOGGER_WIDEN_IF_NEEDED("")),
            m_Sinks(std::make_shared<Sinks>()),
            m_Filter(lvl)
        {
            // 'magic statics'
            StdoutSink::GetGlobalWriteLock();
            Formatter::timestamp_format();
            Formatter::overflow_postfix();
            Formatter::max_length();

            BLOGGER_INIT_UNICODE_MODE();

            if (default_pattern)
            {
                SetPattern(BLOGGER_DEFAULT_PATTERN);
            }
        }

        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        Logger(Logger&& other) = default;
        Logger& operator=(Logger&& other) = default;

        static Ptr CreateFromProps(Props& props);

        static Ptr CreateAsyncConsole(
            BLoggerInString tag,
            level lvl,
            bool default_pattern = true,
            bool colored = true
        );

        static Ptr CreateBlockingConsole(
            BLoggerInString tag,
            level lvl,
            bool default_pattern = true,
            bool colored = true
        );

        void SetPattern(BLoggerInString pattern)
        {
            m_CachedPattern = pattern;
            m_CurrentPattern = m_CachedPattern;
            Formatter::CreatePatternFrom(m_CurrentPattern, m_Tag);
        }

        virtual void Flush() = 0;

        void Log(level lvl, BLoggerInString message)
        {
            if (!ShouldLog(lvl))
                return;

            std::tm time_point;
            auto time_now = std::time(nullptr);
            BLOGGER_UPDATE_TIME(time_point, time_now);

            Post({
                BLoggerString(message.data()),
                m_CurrentPattern.data(),
                time_point,
                lvl
            });
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Log(level lvl, BLoggerInString formattedMsg, Args&& ... args)
        {
            if (!ShouldLog(lvl))
                return;

            std::tm time_point;
            auto time_now = std::time(nullptr);
            BLOGGER_UPDATE_TIME(time_point, time_now);

            Post({
                Formatter::Format(formattedMsg.data(), std::forward<Args>(args)...),
                m_CurrentPattern.data(),
                time_point,
                lvl
            });
        }

       void Trace(BLoggerInString message)
       {
            Log(level::trace, message);
       }

       void Debug(BLoggerInString message)
       {
            Log(level::debug, message);
       }

       void Info(BLoggerInString message)
       {
            Log(level::info, message);
       }

       void Warning(BLoggerInString message)
       {
            Log(level::warn, message);
       }

       void Error(BLoggerInString message)
       {
            Log(level::error, message);
       }

       void Critical(BLoggerInString message)
       {
            Log(level::crit, message);
       }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Trace(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Debug(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Info(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Warning(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Error(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Critical(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::crit, formattedMsg, std::forward<Args>(args)...);
        }

        void SetFilter(level lvl)
        {
            m_Filter = lvl;
        }

        void SetTag(BLoggerInString tag)
        {
            m_Tag = tag;
            SetPattern(m_CachedPattern);
        }

        void AddSink(SinkPtr sink)
        {
            m_Sinks->emplace_back(std::move(sink));
        }

        virtual ~Logger() = default;
    protected:
        bool ShouldLog(level lvl)
        {
            if (m_Filter > lvl)
                return false;

            if (m_Sinks->empty())
                return false;

            if (m_CachedPattern.empty())
                return false;

            return true;
        }

        virtual void Post(BLoggerLogMessage&& msg) = 0;
    };
}
