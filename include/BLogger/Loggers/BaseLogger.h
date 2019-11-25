#pragma once

#include <ctime>
#include <list>

#include "BLogger/Formatter.h"
#include "BLogger/Loggers/LogMessage.h"
#include "BLogger/OS/Functions.h"
#include "BLogger/Sinks/BaseSink.h"
#include "BLogger/LogLevels.h"

#define BLOGGER_INFINITE 0u

#define BLOGGER_DEFAULT_PATTERN BLOGGER_MAKE_UNICODE("[{ts}][{lvl}][{tag}] {msg}")

namespace BLogger {

    typedef std::list<std::unique_ptr<BaseSink>>
        sink_list;
    typedef std::shared_ptr<sink_list>
        BLoggerSharedSinkList;

    class BaseLogger
    {
    protected:
        BLoggerString         m_Tag;
        BLoggerString         m_CachedPattern;
        BLoggerSharedPattern  m_CurrentPattern;
        BLoggerSharedSinkList m_Sinks;
        level                 m_Filter;
    public:
        BaseLogger(
            BLoggerInString tag,
            level lvl,
            bool default_pattern
        ) : m_Tag(tag),
            m_CachedPattern(BLOGGER_MAKE_UNICODE("")),
            m_CurrentPattern(new BLoggerPattern()),
            m_Sinks(new sink_list()),
            m_Filter(lvl)
        {
            if (default_pattern)
            {
                m_CachedPattern = BLOGGER_DEFAULT_PATTERN;
                m_CurrentPattern->init();
                m_CurrentPattern->set_pattern(BLOGGER_DEFAULT_PATTERN, m_Tag);
            }
        }

        BaseLogger(const BaseLogger& other) = delete;
        BaseLogger& operator=(const BaseLogger& other) = delete;

        BaseLogger(BaseLogger&& other) = default;
        BaseLogger& operator=(BaseLogger&& other) = default;

        void SetPattern(BLoggerInString pattern)
        {
            m_CachedPattern = pattern;
            BLoggerPattern* newPattern = new BLoggerPattern();
            newPattern->init();
            newPattern->set_pattern(pattern, m_Tag);

            m_CurrentPattern.reset(newPattern);
        }

        virtual void Flush() = 0;

        void Log(level lvl, BLoggerInString message)
        {
            if (!ShouldLog(lvl))
                return;

            BLoggerFormatter formatter;

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
                lvl
            });
        }

        void Log(level lvl, const bl_char* message)
        {
            if (!ShouldLog(lvl))
                return;

            BLoggerFormatter formatter;

            formatter.process_message(
                message,
                STRING_LENGTH(message)
            );

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(), 
                m_CurrentPattern,
                time_point,
                lvl
            });
        }

        template<typename... Args>
        void Log(level lvl, BLoggerInString formattedMsg, Args&& ... args)
        {
            if (!ShouldLog(lvl))
                return;

            BLoggerFormatter formatter;

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
                lvl
            });
        }

        template<typename... Args>
        void Log(level lvl, const bl_char* formattedMsg, Args&& ... args)
        {
            if (!ShouldLog(lvl))
                return;

            BLoggerFormatter formatter;

            formatter.process_message(
                formattedMsg,
                STRING_LENGTH(formattedMsg)
            );

            BLOGGER_PROCESS_PACK(formatter, args);

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                formatter.release_buffer(),
                m_CurrentPattern,
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
        void Trace(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Debug(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Info(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Warning(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Error(BLoggerInString formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        void Critical(BLoggerInString formattedMsg, Args&& ... args)
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

        void SetTag(BLoggerInString tag)
        {
            m_Tag = tag;
            SetPattern(m_CachedPattern);

            for (auto& sink : *m_Sinks)
            {
                sink->set_tag(tag);
            }
        }

        void AddSink(BaseSink* sink)
        {
            m_Sinks->emplace_back(std::unique_ptr<BaseSink>(sink));
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
