#pragma once

#include "blogger/loggers/log_message.h"

namespace bl {

    inline std::mutex& global_console_write_lock()
    {
        static std::mutex global_write;

        return global_write;
    }

    class sink
    {
    public:
        using ptr = std::unique_ptr<sink>;

        static ptr make_stdout(bool colored = true);

        static ptr make_stderr(bool colored = true);

        static ptr make_stdlog(bool colored = true);

        static ptr make_console(bool colored = true);

        static ptr make_file(
            in_string directory_path,
            size_t bytes_per_file,
            size_t max_log_files,
            bool rotate_logs);

        virtual void write(log_message& msg) = 0;
        virtual void flush() = 0;

        virtual void set_tag(in_string name) {}

        virtual ~sink() = default;
    };
}
