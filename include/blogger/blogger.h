// Main BLogger header

#pragma once

/* Base BLogger class.
   Responsible for message formatting
   and main logging functions.
*/ 
#include "Loggers/Logger.h"

/* Blocking version of BLogger.
   All logging happens on a
   single thread.
*/
#include "Loggers/BlockingLogger.h"

/* Async version of BLogger.
   Logs the mesages using
   a dedicated threadpool
   and a message queue.
*/
#include "Loggers/AsyncLogger.h"

namespace bl {
    inline Sink::Ptr Sink::Stdout(bool colored)
    {
        if (colored)
            return std::make_unique<ColoredStdoutSink>();
        else
            return std::make_unique<StdoutSink>();
    }

    inline Sink::Ptr Sink::Stderr(bool colored)
    {
        if (colored)
            return std::make_unique<ColoredStderrSink>();
        else
            return std::make_unique<StderrSink>();
    }

    inline Sink::Ptr Sink::File(
        InString directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs
    )
    {
        return std::make_unique<FileSink>(
            directoryPath,
            bytesPerFile,
            maxLogFiles,
            rotateLogs
            );
    }

    inline Sink::Ptr Sink::Console(bool colored)
    {
        return Sink::Stderr(colored);
    }

    template<typename... Sinks>
    enable_if_sink_ptr_t<Logger::Ptr, Sinks...> Logger::Custom(
        InString tag,
        level lvl,
        InString pattern,
        bool asynchronous,
        Sinks... sinks)
    {
        Ptr out_logger;

        if (asynchronous)
        {
            // 'magic statics'
            GlobalConsoleWriteLock();
            Formatter::timestamp_format();
            Formatter::overflow_postfix();
            Formatter::max_length();
            Formatter::end();
            thread_pool::get();

            out_logger = std::make_shared<AsyncLogger>(
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

        out_logger->SetPattern(pattern);

        int expander[] = { 0, (out_logger->AddSink(std::move(sinks)), 0) ... };

        return out_logger;
    }

    inline Logger::Ptr Logger::AsyncConsole(
        InString tag,
        level level,
        bool colored
    )
    {
        return Logger::Custom(
            tag,
            level,
            Logger::default_pattern,
            true,
            Sink::Console(colored)
        );
    }

    inline Logger::Ptr Logger::AsyncConsole(
        InString tag,
        level level,
        InString pattern,
        bool colored
    )
    {
        return Logger::Custom(
            tag,
            level,
            pattern,
            true,
            Sink::Console(colored)
        );
    }

    inline Logger::Ptr Logger::Console(
        InString tag,
        level level,
        bool colored
    )
    {
        return Logger::Custom(
            tag,
            level,
            Logger::default_pattern,
            false,
            Sink::Console(colored)
        );
    }

    inline Logger::Ptr Logger::Console(
        InString tag,
        level level,
        InString pattern,
        bool colored
    )
    {
        return Logger::Custom(
            tag,
            level,
            pattern,
            false,
            Sink::Console(colored)
        );
    }

    inline Logger::Ptr File(
        InString tag,
        level level,
        InString pattern,
        InString directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs = true
    )
    {
        return Logger::Custom(
            tag,
            level,
            pattern,
            false,
            Sink::File(
                directoryPath,
                bytesPerFile,
                maxLogFiles,
                rotateLogs
            )
        );
    }

    inline Logger::Ptr AsyncFile(
        InString tag,
        level level,
        InString pattern,
        InString directoryPath,
        size_t bytesPerFile,
        size_t maxLogFiles,
        bool rotateLogs = true
    )
    {
        return Logger::Custom(
            tag,
            level,
            Logger::default_pattern,
            true,
            Sink::File(
                directoryPath,
                bytesPerFile,
                maxLogFiles,
                rotateLogs
            )
        );
    }
}

#undef BLOGGER_ARG_CLOSING
#undef BLOGGER_ARG_FULL
#undef BLOGGER_ARG_OPENING
#undef BLOGGER_DEFAULT_PATTERN
#undef BLOGGER_LVL_PATTERN
#undef BLOGGER_TS_PATTERN
#undef BLOGGER_MSG_PATTERN
#undef BLOGGER_TAG_PATTERN
#undef BLOGGER_TIMESTAMP_FORMAT
#undef BLOGGER_FILEMODE
#undef BLOGGER_FILE_WRITE
#undef BLOGGER_FORMAT_STRING
#undef BLOGGER_STD_TO_STRING
#undef BLOGGER_TIME_TO_STRING
#undef BLOGGER_STRING_LENGTH
#undef BLOGGER_TO_STRING
#undef BLOGGER_STACK_ALLOC
#undef BLOGGER_MEMORY_COPY
#undef BLOGGER_MEMORY_MOVE
#undef BLOGGER_OPEN_FILE
#undef BLOGGER_OVERFLOW_POSTFIX
#undef BLOGGER_FOR_EACH_DO
#undef BLOGGER_TERMINATE_WITH
#undef BLOGGER_TRUE_SIZE
#undef BLOGGER_UPDATE_TIME
#undef BLOGGER_INIT_UNICODE_MODE
