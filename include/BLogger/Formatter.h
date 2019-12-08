#pragma once

#include <memory>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#include "BLogger/OS/Functions.h"
#include "BLogger/LogLevels.h"

#define BLOGGER_ARG_OPENING BLOGGER_WIDEN_IF_NEEDED("{")
#define BLOGGER_ARG_CLOSING BLOGGER_WIDEN_IF_NEEDED("}")
#define BLOGGER_ARG_FULL BLOGGER_WIDEN_IF_NEEDED("{}")

#define BLOGGER_TS_PATTERN  BLOGGER_WIDEN_IF_NEEDED("{ts}")
#define BLOGGER_TAG_PATTERN BLOGGER_WIDEN_IF_NEEDED("{tag}")
#define BLOGGER_LVL_PATTERN BLOGGER_WIDEN_IF_NEEDED("{lvl}")
#define BLOGGER_MSG_PATTERN BLOGGER_WIDEN_IF_NEEDED("{msg}")

#define BLOGGER_TIMESTAMP_FORMAT BLOGGER_WIDEN_IF_NEEDED("%H:%M:%S")

#define BLOGGER_TERMINATE_WITH BLOGGER_WIDEN_IF_NEEDED('\n')
#define BLOGGER_OVERFLOW_POSTFIX BLOGGER_WIDEN_IF_NEEDED("...")

class CreateLogger;

namespace BLogger
{
    class Formatter
    {
    private:
        friend class ::CreateLogger;
        friend class BaseLogger;
    public:
        static void CreatePatternFrom(
            BLoggerString& out_pattern,
            BLoggerInString tag
        )
        {
            if (out_pattern.empty())
                return;

            find_and_replace(out_pattern, BLOGGER_TAG_PATTERN, tag);
        }

        template<typename... Args>
        static BLoggerString Format(BLoggerString pattern, Args&& ... args)
        {
            uint16_t index = 0;
            BLOGGER_FOR_EACH_DO(format_one, args, pattern, index);
            return pattern;
        }

        static void MergePattern(
            BLoggerString& formatted_msg,
            BLoggerString& merge_into,
            std::tm* time_ptr,
            level lvl
        )
        {
            find_and_replace(merge_into, BLOGGER_MSG_PATTERN, formatted_msg);
            find_and_replace(merge_into, BLOGGER_TS_PATTERN, timestamp_format().c_str());
            find_and_replace_timestamp(merge_into, timestamp_format().c_str(), time_ptr);
            find_and_replace_level(merge_into, BLOGGER_LVL_PATTERN, lvl);

            if (max_length() != BLOGGER_INFINITE &&
                merge_into.size() > max_length()
            )
            {
                size_t to_cut =
                    merge_into.size() -
                    max_length() +
                    overflow_postfix().size();

                merge_into.resize(merge_into.size() - to_cut);
                merge_into += overflow_postfix();
            }

            merge_into.push_back(BLOGGER_TERMINATE_WITH);
        }

        static void CutIfExceeds(
            uint64_t length,
            BLoggerString overflow_postfix = BLOGGER_OVERFLOW_POSTFIX
        )
        {
            max_length() = length;
            ::BLogger::Formatter::overflow_postfix() = overflow_postfix;
        }

        // Uses strftime format - https://en.cppreference.com/w/cpp/chrono/c/strftime
        static void SetTimestampFormat(BLoggerString new_format = BLOGGER_TIMESTAMP_FORMAT)
        {
            timestamp_format() = new_format;
        }
    private:
        template<typename T>
        static void find_and_replace(BLoggerString& in, BLoggerInString what, T&& with)
        {
            auto pos = in.find(what);
            if (pos == BLoggerString::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, to_string(with).c_str());
        }

        static void find_and_replace_timestamp(BLoggerString& in, BLoggerInString what, std::tm* time)
        {
            auto pos = in.find(what);
            if (pos == BLoggerString::npos) return;

            // If your timestamp is longer than this
            // then you're doing something wrong...
            constexpr size_t ts_size = 128;

            bl_char timestamp[ts_size];

            auto written = BLOGGER_TIME_TO_STRING(timestamp, ts_size, timestamp_format().c_str(), time);

            in.erase(pos, timestamp_format().size());

            if (!written) return;

            in.insert(pos, timestamp);
        }

        static void find_and_replace_level(BLoggerString& in, BLoggerInString what, level lvl)
        {
            auto pos = in.find(what);
            if (pos == BLoggerString::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, LevelToString(lvl));
        }

        template<typename T>
        static void format_one(BLoggerString& out_formatted, uint16_t& index, T&& arg)
        {
            BLoggerString stringed_arg = to_string(arg);

            BLoggerString pos_arg;
            pos_arg.reserve(6); // reserve slightly more than we actually expect
            pos_arg += BLOGGER_ARG_OPENING;
            pos_arg += to_string(index);
            pos_arg += BLOGGER_ARG_CLOSING;

            auto arg_offset = out_formatted.find(pos_arg);
            if (arg_offset == BLoggerString::npos)
            {
                arg_offset = out_formatted.find(BLOGGER_ARG_FULL);
                pos_arg = BLOGGER_ARG_FULL;
            }
            if (arg_offset == BLoggerString::npos)
                return;

            index++;

            out_formatted.erase(arg_offset, pos_arg.size());
            out_formatted.insert(arg_offset, stringed_arg.c_str());
        }

        static BLoggerString& overflow_postfix()
        {
            static BLoggerString overflow_postfix = BLOGGER_OVERFLOW_POSTFIX;
            return overflow_postfix;
        }

        static uint64_t& max_length()
        {
            static uint64_t length = BLOGGER_INFINITE;
            return length;
        }

        static BLoggerString& timestamp_format()
        {
            static BLoggerString timestamp_format = BLOGGER_TIMESTAMP_FORMAT;
            return timestamp_format;
        }
    };
}
