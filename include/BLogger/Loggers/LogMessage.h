#pragma once

#include <bitset>
#include "BLogger/LogLevels.h"
#include "BLogger/Formatter/Formatter.h"

namespace BLogger {

    struct BLoggerLogMessage
    {
    private:
        bl_string formatted_msg;
        BLoggerSharedPattern ptrn;
        std::tm time_point;
        level lvl;

        // stdout, file, color
        std::bitset<3> props;
    public:
        BLoggerLogMessage(
            bl_string&& formatted_msg,
            BLoggerSharedPattern& ptrn,
            std::tm tp,
            level lvl,
            bool log_stdout,
            bool log_file,
            bool colored,
            uint16_t sender_id
        )
            : formatted_msg(std::move(formatted_msg)),
            ptrn(ptrn),
            time_point(tp),
            lvl(lvl)
        {
            props[0] = log_stdout;
            props[1] = log_file;
            props[2] = colored;
        }

        void finalize_format()
        {
            BLoggerFormatter::merge_pattern(formatted_msg, ptrn, time_point_ptr(), lvl);
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

        bool colored()
        {
            return props[2];
        }

        bool console_logger()
        {
            return props[0];
        }

        bool file_logger()
        {
            return props[1];
        }

        uint16_t sender()
        {
            return ptrn->owner();
        }
    private:
        std::tm* time_point_ptr()
        {
            return &time_point;
        }
    };
}
