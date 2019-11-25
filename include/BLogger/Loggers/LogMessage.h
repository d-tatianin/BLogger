#pragma once

#include "BLogger/Formatter.h"
#include "BLogger/LogLevels.h"

namespace BLogger {

    struct BLoggerLogMessage
    {
    private:
        bl_string formatted_msg;
        BLoggerSharedPattern ptrn;
        std::tm time_point;
        level lvl;
    public:
        BLoggerLogMessage(
            bl_string&& formatted_msg,
            BLoggerSharedPattern& ptrn,
            std::tm tp,
            level lvl
        ) : formatted_msg(std::move(formatted_msg)),
            ptrn(ptrn),
            time_point(tp),
            lvl(lvl)
        {
        }

        void finalize_format()
        {
            BLoggerFormatter::merge_pattern(
                formatted_msg,
                ptrn,
                time_point_ptr(),
                lvl
            );
        }

        bl_char* data()
        {
            return formatted_msg.data();
        }

        size_t size()
        {
            return formatted_msg.size();
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
