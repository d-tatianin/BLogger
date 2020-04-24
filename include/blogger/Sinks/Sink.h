#pragma once

#include "blogger/Loggers/LogMessage.h"

namespace bl {

    inline std::mutex& GlobalConsoleWriteLock()
    {
        static std::mutex globalWrite;

        return globalWrite;
    }

    class Sink
    {
    public:
        using Ptr = std::unique_ptr<Sink>;

        static Ptr Stdout(bool colored = true);

        static Ptr Stderr(bool colored = true);

        static Ptr Console(bool colored = true);

        static Ptr File(
            InString directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true);


        virtual void write(LogMessage& msg) = 0;
        virtual void flush() = 0;

        virtual void set_name(InString name) {}

        virtual ~Sink() = default;
    };
}
