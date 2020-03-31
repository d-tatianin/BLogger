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
    Logger::Ptr Logger::CreateFromProps(Props& props)
    {
        Ptr out_logger;

        if (props.async)
        {
            // 'magic statics'
            StdoutSink::GetGlobalWriteLock();
            Formatter::timestamp_format();
            Formatter::overflow_postfix();
            Formatter::max_length();
            Formatter::end();
            thread_pool::get();

            out_logger = std::make_shared<AsyncLogger>(
                props.tag,
                props.filter,
                props.pattern.empty()
                );
        }
        else
            out_logger = std::make_shared<BlockingLogger>(
                props.tag,
                props.filter,
                props.pattern.empty()
                );

        if (!props.pattern.empty())
            out_logger->SetPattern(props.pattern);

        if (props.tag.empty()) props.tag = BLOGGER_WIDEN_IF_NEEDED("Unnamed");

        if (props.console_logger)
        {
            if (props.colored)
                out_logger->AddSink(
                    std::make_unique<bl::ColoredStdoutSink>()
                );
            else
                out_logger->AddSink(
                    std::make_unique<bl::StdoutSink>()
                );
        }

        if (props.file_logger)
        {
            if (!props.path.empty())
            {
                out_logger->AddSink(
                    std::make_unique<bl::FileSink>(
                        props.path, props.tag,
                        props.bytes_per_file,
                        props.log_files,
                        props.rotate_logs
                        )
                );
            }
        }

        return out_logger;
    }

    Logger::Ptr Logger::CreateAsyncConsole(
        InString tag,
        level lvl,
        bool default_pattern,
        bool colored
    )
    {
        // 'magic statics'
        StdoutSink::GetGlobalWriteLock();
        Formatter::timestamp_format();
        Formatter::overflow_postfix();
        Formatter::max_length();
        Formatter::end();
        thread_pool::get();

        Ptr out_logger =
            std::make_shared<AsyncLogger>(
                tag,
                lvl,
                default_pattern
                );

        if (colored)
            out_logger->AddSink(
                std::make_unique<bl::ColoredStdoutSink>()
            );
        else
            out_logger->AddSink(
                std::make_unique<bl::StdoutSink>()
            );

        return out_logger;
    }

    Logger::Ptr Logger::CreateBlockingConsole(
        InString tag,
        level lvl,
        bool default_pattern,
        bool colored
    )
    {
        Ptr out_logger =
            std::make_shared<BlockingLogger>(
                tag,
                lvl,
                default_pattern
                );

        if (colored)
            out_logger->AddSink(
                std::make_unique<bl::ColoredStdoutSink>()
            );
        else
            out_logger->AddSink(
                std::make_unique<bl::StdoutSink>()
            );

        return out_logger;
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
