#pragma once

#include <ctime>

#include "blogger/Formatter.h"
#include "blogger/Loggers/LogMessage.h"
#include "blogger/OS/Functions.h"
#include "blogger/Sinks/Sink.h"
#include "blogger/Sinks/ConsoleSink.h"
#include "blogger/LogLevels.h"

namespace bl {

    using Sinks = std::vector<Sink::Ptr>;
    using SharedSinks = std::shared_ptr<Sinks>;

    template<typename T>
    struct is_sink_ptr : public std::false_type
    {
    };

    template<>
    struct is_sink_ptr<Sink::Ptr> : public std::true_type
    {
    };

    template<typename T, typename... Args>
    using enable_if_sink_ptr = std::enable_if<are_all_true<is_sink_ptr<Args>...>::value, T>;

    template<typename T, typename... Args>
    using enable_if_sink_ptr_t = typename std::enable_if<are_all_true<is_sink_ptr<Args>...>::value, T>::type;

    class Logger
    {
    protected:
        String         m_Tag;
        String         m_CurrentPattern;
        String         m_CachedPattern;
        SharedSinks    m_Sinks;
        level          m_Filter;
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
            GlobalConsoleWriteLock();
            Formatter::timestamp_format();
            Formatter::overflow_postfix();
            Formatter::max_length();

            BLOGGER_INIT_UNICODE_MODE();

            if (default_pattern)
                SetPattern(Logger::default_pattern);
        }

        Logger(const Logger& other) = delete;
        Logger& operator=(const Logger& other) = delete;

        Logger(Logger&& other) = default;
        Logger& operator=(Logger&& other) = default;

        template<typename... Sinks>
        static enable_if_sink_ptr_t<Ptr, Sinks...> Custom(
            InString tag,
            level level,
            InString pattern,
            bool asynchronous,
            Sinks... sinks
        );

        static Ptr AsyncConsole(
            InString tag,
            level level,
            bool colored = true
        );

        static Ptr AsyncConsole(
            InString tag,
            level level,
            InString pattern,
            bool colored = true
        );

        static Ptr Console(
            InString tag,
            level level,
            bool colored = true
        );

        static Ptr Console(
            InString tag,
            level level,
            InString pattern,
            bool colored = true
        );

        static Ptr File(
            InString tag,
            level level,
            InString pattern,
            InString directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true
        );

        static Ptr AsyncFile(
            InString tag,
            level level,
            InString pattern,
            InString directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true
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

            SetSinkTags();
        }

        void AddSink(Sink::Ptr sink)
        {
            sink->set_name(m_Tag);

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

        virtual void Post(LogMessage&& msg) = 0;

    private:
        void SetSinkTags()
        {
            for (auto& sink : *m_Sinks)
                sink->set_name(m_Tag);
        }
    };
}
