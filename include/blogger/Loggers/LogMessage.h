#pragma once

#include "blogger/Formatter.h"
#include "blogger/LogLevels.h"

namespace bl {

    struct LogMessage
    {
    private:
        String formatted_msg;
        String final_pattern;
        std::tm time_point;
        level lvl;
    public:
        LogMessage(
            String&& formatted_msg,
            String&& ptrn,
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

        const char_t* data()
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
