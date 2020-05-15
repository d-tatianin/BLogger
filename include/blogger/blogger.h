// Main BLogger header

#pragma once

/* Base BLogger class.
   Responsible for message formatting
   and main logging functions.
*/ 
#include "loggers/logger.h"

/* Blocking version of BLogger.
   All logging happens on a
   single thread.
*/
#include "loggers/blocking_logger.h"

/* Async version of BLogger.
   Logs the mesages using
   a dedicated threadpool
   and a message queue.
*/
#include "loggers/async_logger.h"

namespace bl {
    inline sink::ptr sink::make_stdout(bool colored)
    {
        if (colored)
            return std::make_unique<colored_stdout_sink>();
        else
            return std::make_unique<stdout_sink>();
    }

    inline sink::ptr sink::make_stderr(bool colored)
    {
        if (colored)
            return std::make_unique<colored_stderr_sink>();
        else
            return std::make_unique<stderr_sink>();
    }

    inline sink::ptr sink::make_stdlog(bool colored)
    {
        if (colored)
            return std::make_unique<colored_stdlog_sink>();
        else
            return std::make_unique<stdlog_sink>();
    }

    inline sink::ptr sink::make_syslog()
    {
        return std::make_unique<syslog_sink>();
    }

    inline sink::ptr sink::make_file(
        in_string directory_path,
        size_t bytes_per_file,
        size_t max_log_files,
        bool rotate_logs
    )
    {
        return std::make_unique<file_sink>(
            directory_path,
            bytes_per_file,
            max_log_files,
            rotate_logs
        );
    }

    inline sink::ptr sink::make_console(bool colored)
    {
        return sink::make_stdlog(colored);
    }

    template<typename... Sinks>
    enable_if_sink_ptr_t<logger::ptr, Sinks...> logger::make_custom(
        in_string tag,
        level lvl,
        in_string pattern,
        bool asynchronous,
        Sinks... sinks)
    {
        ptr out_logger;

        if (asynchronous)
        {
            // 'magic statics'
            global_console_write_lock();
            formatter::timestamp_format();
            formatter::overflow_postfix();
            formatter::max_length();
            formatter::ending();
            thread_pool::get();

            out_logger = std::make_shared<async_logger>(
                tag,
                lvl,
                false
            );
        }
        else
            out_logger = std::make_shared<blocking_logger>(
                tag,
                lvl,
                false
            );

        out_logger->set_pattern(pattern);

        // (MSVC) ignore the E1919 here
        BLOGGER_FOR_EACH_DO(out_logger->add_sink, Sinks, std::move(sinks));

        return out_logger;
    }

    inline logger::ptr logger::make_async_console(
        in_string tag,
        level lvl,
        in_string pattern,
        bool colored
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            true,
            sink::make_console(colored)
        );
    }

    inline logger::ptr logger::make_console(
        in_string tag,
        level lvl,
        in_string pattern,
        bool colored
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            false,
            sink::make_console(colored)
        );
    }

    inline logger::ptr logger::make_file(
        in_string tag,
        level lvl,
        in_string pattern,
        in_string directory_path,
        size_t bytes_per_file,
        size_t max_log_files,
        bool rotate_logs
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            false,
            sink::make_file(
                directory_path,
                bytes_per_file,
                max_log_files,
                rotate_logs
            )
        );
    }

    inline logger::ptr logger::make_async_file(
        in_string tag,
        level lvl,
        in_string pattern,
        in_string directory_path,
        size_t bytes_per_file,
        size_t max_log_files,
        bool rotate_logs
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            true,
            sink::make_file(
                directory_path,
                bytes_per_file,
                max_log_files,
                rotate_logs
            )
        );
    }
}

#undef BLOGGER_FILEMODE
#undef BLOGGER_OPEN_FILE
#undef BLOGGER_FILE_WRITE

#undef BLOGGER_INIT_UNICODE_MODE
#undef BLOGGER_STRING_LENGTH
#undef BLOGGER_TIME_TO_STRING
#undef BLOGGER_STD_TO_STRING
#undef BLOGGER_TO_STRING

#undef BLOGGER_TRUE_SIZE
#undef BLOGGER_FOR_EACH_DO
#undef BLOGGER_VA_FOR_EACH_DO
#undef BLOGGER_UPDATE_TIME

#undef BLOGGER_STACK_ALLOC
