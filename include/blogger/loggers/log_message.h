#pragma once

#include "blogger/formatter.h"
#include "blogger/log_levels.h"

namespace bl {

    struct log_message
    {
    private:
        string formatted_msg;
        string final_pattern;
        std::tm time_point;
        level lvl;
    public:
        log_message(
            string&& formatted_msg,
            string&& ptrn,
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
            formatter::merge_pattern(
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
