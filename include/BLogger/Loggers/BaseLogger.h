#pragma once

#include <ctime>

#include "BLogger/Formatter.h"
#include "BLogger/Loggers/LogMessage.h"
#include "BLogger/OS/Functions.h"
#include "BLogger/Sinks/BaseSink.h"
#include "BLogger/Sinks/StdoutSink.h"
#include "BLogger/LogLevels.h"

#define BLOGGER_DEFAULT_PATTERN BLOGGER_WIDEN_IF_NEEDED("[{ts}][{lvl}][{tag}] {msg}")

namespace BLogger {

    typedef std::unique_ptr<BaseSink>
        BLoggerSinkPtr;
    typedef std::vector<BLoggerSinkPtr>
        BLoggerSinks;
    typedef std::shared_ptr<BLoggerSinks>
        BLoggerSharedSinks;

    class BaseLogger
    {
    protected:
        BLoggerString         m_Tag;
        BLoggerString         m_CachedPattern;
        BLoggerSharedSinks    m_Sinks;
        level                 m_Filter;
    public:
        BaseLogger(
            BLoggerInString tag,
            level lvl,
            bool default_pattern
        ) : m_Tag(tag),
            m_CachedPattern(BLOGGER_WIDEN_IF_NEEDED("")),
            m_Sinks(std::make_shared<BLoggerSinks>()),
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

        BaseLogger(const BaseLogger& other) = delete;
        BaseLogger& operator=(const BaseLogger& other) = delete;

        BaseLogger(BaseLogger&& other) = default;
        BaseLogger& operator=(BaseLogger&& other) = default;

        void SetPattern(BLoggerInString pattern)
        {
            m_CachedPattern = pattern;
            Formatter::CreatePatternFrom(m_CachedPattern, m_Tag);
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
                m_CachedPattern.data(),
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
                m_CachedPattern.data(),
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

            for (auto& sink : *m_Sinks)
            {
                sink->set_tag(tag);
            }
        }

        void AddSink(BLoggerSinkPtr sink)
        {
            m_Sinks->emplace_back(std::move(sink));
            m_Sinks->back()->set_tag(m_Tag);
        }

        virtual ~BaseLogger() {}
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
