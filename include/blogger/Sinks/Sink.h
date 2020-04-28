#pragma once

#include "blogger/loggers/log_message.h"

namespace bl {

    inline std::mutex& global_console_write_lock()
    {
        static std::mutex globalWrite;

        return globalWrite;
    }

    class sink
    {
    public:
        using ptr = std::unique_ptr<sink>;

        static ptr make_stdout(bool colored = true);

        static ptr make_stderr(bool colored = true);

        static ptr make_console(bool colored = true);

        static ptr make_file(
            in_string directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true);

        virtual void write(log_message& msg) = 0;
        virtual void flush() = 0;

        virtual void set_tag(in_string name) {}

        virtual ~sink() = default;
    };
}
