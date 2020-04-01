#pragma once

#include <ctime>

#include "blogger/Formatter.h"
#include "blogger/Loggers/LogMessage.h"
#include "blogger/OS/Functions.h"
#include "blogger/Sinks/Sink.h"
#include "blogger/Sinks/StdoutSink.h"
#include "blogger/LogLevels.h"

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
        String tag;
        String pattern;
        level filter;

        bool file_logger;
        String path;
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
            bytes_per_file(infinite),
            log_files(0),
            rotate_logs(true)
        {
        }
    };

    class Logger
    {
    protected:
        String         m_Tag;
        String         m_CurrentPattern;
        String         m_CachedPattern;
        SharedSinks           m_Sinks;
        level                 m_Filter;
    public:
        static auto constexpr default_pattern = BLOGGER_WIDEN_IF_NEEDED("[{ts}][{lvl}][{tag}] {msg}");
        using Ptr = std::shared_ptr<Logger>;

        Logger(
            InString tag,
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
                SetPattern(Logger::default_pattern);
            }
        }

        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        Logger(Logger&& other) = default;
        Logger& operator=(Logger&& other) = default;

        static Ptr CreateFromProps(Props& props);

        static Ptr CreateAsyncConsole(
            InString tag,
            level lvl,
            bool default_pattern = true,
            bool colored = true
        );

        static Ptr CreateBlockingConsole(
            InString tag,
            level lvl,
            bool default_pattern = true,
            bool colored = true
        );

        void SetPattern(InString pattern)
        {
            m_CachedPattern = pattern;
            m_CurrentPattern = m_CachedPattern;
            Formatter::CreatePatternFrom(m_CurrentPattern, m_Tag);
        }

        virtual void Flush() = 0;

        void Log(level lvl, InString message)
        {
            if (!ShouldLog(lvl))
                return;

            std::tm time_point;
            auto time_now = std::time(nullptr);
            BLOGGER_UPDATE_TIME(time_point, time_now);

            Post({
                String(message.data()),
                m_CurrentPattern.data(),
                time_point,
                lvl
            });
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Log(level lvl, InString formattedMsg, Args&& ... args)
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

       void Trace(InString message)
       {
            Log(level::trace, message);
       }

       void Debug(InString message)
       {
            Log(level::debug, message);
       }

       void Info(InString message)
       {
            Log(level::info, message);
       }

       void Warning(InString message)
       {
            Log(level::warn, message);
       }

       void Error(InString message)
       {
            Log(level::error, message);
       }

       void Critical(InString message)
       {
            Log(level::crit, message);
       }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Trace(InString formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Debug(InString formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Info(InString formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Warning(InString formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Error(InString formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> Critical(InString formattedMsg, Args&& ... args)
        {
            Log(level::crit, formattedMsg, std::forward<Args>(args)...);
        }

        void SetFilter(level lvl)
        {
            m_Filter = lvl;
        }

        void SetTag(InString tag)
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
