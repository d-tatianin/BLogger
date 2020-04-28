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

    inline sink::ptr sink::make_file(
        in_string directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs
    )
    {
        return std::make_unique<file_sink>(
            directoryPath,
            bytesPerFile,
            maxLogFiles,
            rotateLogs
        );
    }

    inline sink::ptr sink::make_console(bool colored)
    {
        return sink::make_stderr(colored);
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
            out_logger = std::make_shared<BlockingLogger>(
                tag,
                lvl,
                false
            );

        out_logger->set_pattern(pattern);

        int expander[] = { 0, (out_logger->add_sink(std::move(sinks)), 0) ... };

        return out_logger;
    }

    inline logger::ptr logger::make_async_console(
        in_string tag,
        level lvl,
        bool colored
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            logger::default_pattern,
            true,
            sink::make_console(colored)
        );
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
        bool colored
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            logger::default_pattern,
            false,
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
        in_string directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            false,
            sink::make_file(
                directoryPath,
                bytesPerFile,
                maxLogFiles,
                rotateLogs
            )
        );
    }

    inline logger::ptr logger::make_async_file(
        in_string tag,
        level lvl,
        in_string pattern,
        in_string directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs
    )
    {
        return logger::make_custom(
            tag,
            lvl,
            pattern,
            true,
            sink::make_file(
                directoryPath,
                bytesPerFile,
                maxLogFiles,
                rotateLogs
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
#undef BLOGGER_UPDATE_TIME

#undef BLOGGER_STACK_ALLOC
