#pragma once

#include <ctime>

#include "blogger/formatter.h"
#include "blogger/loggers/log_message.h"
#include "blogger/os/functions.h"
#include "blogger/sinks/sink.h"
#include "blogger/log_levels.h"

namespace bl {

    using sinks = std::vector<sink::ptr>;
    using shared_sinks = std::shared_ptr<sinks>;

    template<typename T>
    struct is_sink_ptr : public std::false_type
    {
    };

    template<>
    struct is_sink_ptr<sink::ptr> : public std::true_type
    {
    };

    template<typename T, typename... Args>
    using enable_if_sink_ptr = std::enable_if<are_all_true<is_sink_ptr<Args>...>::value, T>;

    template<typename T, typename... Args>
    using enable_if_sink_ptr_t = typename std::enable_if<are_all_true<is_sink_ptr<Args>...>::value, T>::type;

    class logger
    {
    protected:
        string         m_tag;
        string         m_current_pattern;
        string         m_cached_pattern;
        shared_sinks   m_sinks;
        level          m_filter;
    public:
        static auto constexpr default_pattern = BLOGGER_WIDEN_IF_NEEDED("[{ts}][{lvl}][{tag}] {msg}");
        static auto constexpr default_tag     = BLOGGER_WIDEN_IF_NEEDED("Unnamed");
        static auto constexpr default_path    = BLOGGER_WIDEN_IF_NEEDED("logs");
        using ptr = std::shared_ptr<logger>;

        logger(
            in_string tag,
            level lvl,
            bool default_pattern
        ) : m_tag(tag),
            m_cached_pattern(BLOGGER_WIDEN_IF_NEEDED("")),
            m_sinks(std::make_shared<sinks>()),
            m_filter(lvl)
        {
            // 'magic statics'
            global_console_write_lock();
            formatter::timestamp_format();
            formatter::overflow_postfix();
            formatter::max_length();

            init_unicode();

            if (default_pattern)
                set_pattern(logger::default_pattern);
        }

        logger(const logger& other) = delete;
        logger& operator=(const logger& other) = delete;

        logger(logger&& other) = default;
        logger& operator=(logger&& other) = default;

        template<typename... Sinks>
        static enable_if_sink_ptr_t<ptr, Sinks...> make_custom(
            in_string tag,
            level lvl,
            in_string pattern,
            bool asynchronous,
            Sinks... sinks
        );

        static ptr make_async_console(
            in_string tag = default_tag,
            level lvl = level::info,
            in_string pattern = default_pattern,
            bool colored = true
        );

        static ptr make_console(
            in_string tag = default_tag,
            level lvl = level::info,
            in_string pattern = default_pattern,
            bool colored = true
        );

        static ptr make_file(
            in_string tag = default_tag,
            level lvl = level::info,
            in_string pattern = default_pattern,
            in_string directory_path = default_path,
            size_t bytes_per_file = infinite,
            size_t max_log_files = infinite,
            bool rotate_logs = true
        );

        static ptr make_async_file(
            in_string tag = default_tag,
            level lvl = level::info,
            in_string pattern = default_pattern,
            in_string directory_path = default_path,
            size_t bytes_per_file = infinite,
            size_t max_log_files = infinite,
            bool rotate_logs = true
        );

        void set_pattern(in_string pattern)
        {
            m_cached_pattern = pattern;
            m_current_pattern = m_cached_pattern;
            formatter::create_pattern_from(m_current_pattern, m_tag);
        }

        virtual void flush() = 0;

        void log(level lvl, in_string message)
        {
            if (!should_log(lvl))
                return;

            std::tm time_point;
            auto time_now = std::time(nullptr);
            BLOGGER_UPDATE_TIME(time_point, time_now);

            post({
                string(message.data()),
                m_current_pattern.data(),
                time_point,
                lvl
            });
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> log(level lvl, in_string formatted_msg, Args&& ... args)
        {
            if (!should_log(lvl))
                return;

            std::tm time_point;
            auto time_now = std::time(nullptr);
            BLOGGER_UPDATE_TIME(time_point, time_now);

            post({
                formatter::format(formatted_msg.data(), std::forward<Args>(args)...),
                m_current_pattern.data(),
                time_point,
                lvl
            });
        }

       void trace(in_string message)
       {
            log(level::trace, message);
       }

       void debug(in_string message)
       {
            log(level::debug, message);
       }

       void info(in_string message)
       {
            log(level::info, message);
       }

       void warning(in_string message)
       {
            log(level::warn, message);
       }

       void error(in_string message)
       {
            log(level::error, message);
       }

       void critical(in_string message)
       {
            log(level::crit, message);
       }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> trace(in_string formatted_msg, Args&& ... args)
        {
            log(level::trace, formatted_msg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> debug(in_string formatted_msg, Args&& ... args)
        {
            log(level::debug, formatted_msg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> info(in_string formatted_msg, Args&& ... args)
        {
            log(level::info, formatted_msg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> warning(in_string formatted_msg, Args&& ... args)
        {
            log(level::warn, formatted_msg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> error(in_string formatted_msg, Args&& ... args)
        {
            log(level::error, formatted_msg, std::forward<Args>(args)...);
        }

        template<typename... Args>
        enable_if_ostream_insertable_t<Args...> critical(in_string formatted_msg, Args&& ... args)
        {
            log(level::crit, formatted_msg, std::forward<Args>(args)...);
        }

        void set_filter(level lvl)
        {
            m_filter = lvl;
        }

        void set_tag(in_string tag)
        {
            m_tag = tag;
            set_pattern(m_cached_pattern);

            set_sinks_tag();
        }

        void add_sink(sink::ptr sink)
        {
            sink->set_tag(m_tag);

            m_sinks->emplace_back(std::move(sink));
        }

        virtual ~logger() = default;
    protected:
        bool should_log(level lvl)
        {
            if (m_filter > lvl)
                return false;

            if (m_sinks->empty())
                return false;

            if (m_cached_pattern.empty())
                return false;

            return true;
        }

        virtual void post(log_message&& msg) = 0;

    private:
        void set_sinks_tag()
        {
            for (auto& sink : *m_sinks)
                sink->set_tag(m_tag);
        }
    };
}
