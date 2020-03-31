#pragma once

#include "BLogger/Formatter.h"
#include "BLogger/LogLevels.h"

namespace bl {

    struct BLoggerLogMessage
    {
    private:
        BLoggerString formatted_msg;
        BLoggerString final_pattern;
        std::tm time_point;
        level lvl;
    public:
        BLoggerLogMessage(
            BLoggerString&& formatted_msg,
            BLoggerString&& ptrn,
            std::tm tp,
            level lvl
        ) : formatted_msg(std::move(formatted_msg)),
            final_pattern(std::move(ptrn)),
            time_point(tp),
            lvl(lvl)
        {
        }

        void finalize_format()
        {
            Formatter::MergePattern(
                formatted_msg,
                final_pattern,
                time_point_ptr(),
                lvl
            );
        }

        const bl_char* data()
        {
            return final_pattern.data();
        }

        size_t size()
        {
            return final_pattern.size();
        }

        level log_level()
        {
            return lvl;
        }
    private:
        std::tm* time_point_ptr()
        {
            return &time_point;
        }
    };
}
