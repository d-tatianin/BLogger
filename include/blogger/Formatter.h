#pragma once

#include <memory>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#include "blogger/OS/Functions.h"
#include "blogger/LogLevels.h"

#define BLOGGER_ARG_OPENING BLOGGER_WIDEN_IF_NEEDED("{")
#define BLOGGER_ARG_CLOSING BLOGGER_WIDEN_IF_NEEDED("}")
#define BLOGGER_ARG_FULL BLOGGER_WIDEN_IF_NEEDED("{}")

#define BLOGGER_TS_PATTERN  BLOGGER_WIDEN_IF_NEEDED("{ts}")
#define BLOGGER_TAG_PATTERN BLOGGER_WIDEN_IF_NEEDED("{tag}")
#define BLOGGER_LVL_PATTERN BLOGGER_WIDEN_IF_NEEDED("{lvl}")
#define BLOGGER_MSG_PATTERN BLOGGER_WIDEN_IF_NEEDED("{msg}")

#define BLOGGER_TIMESTAMP_FORMAT BLOGGER_WIDEN_IF_NEEDED("%H:%M:%S")

#define BLOGGER_TERMINATE_WITH BLOGGER_WIDEN_IF_NEEDED("\n")
#define BLOGGER_OVERFLOW_POSTFIX BLOGGER_WIDEN_IF_NEEDED("...")

class CreateLogger;

namespace bl
{
    class Formatter
    {
        friend class Logger;
    public:
        static void CreatePatternFrom(
            String& out_pattern,
            InString tag
        )
        {
            if (out_pattern.empty())
                return;

            find_and_replace(out_pattern, BLOGGER_TAG_PATTERN, tag);
        }

        template<typename... Args>
        static String Format(String pattern, Args&& ... args)
        {
            uint16_t index = 0;
            BLOGGER_FOR_EACH_DO(format_one, args, pattern, index);
            return pattern;
        }

        static void MergePattern(
            String& formatted_msg,
            String& merge_into,
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

            merge_into += end();
        }

        static void CutIfExceeds(
            uint64_t length,
            InString postfix = BLOGGER_OVERFLOW_POSTFIX
        )
        {
            max_length() = length;
            overflow_postfix() = postfix;
        }

        // Uses strftime format - https://en.cppreference.com/w/cpp/chrono/c/strftime
        static void SetTimestampFormat(InString new_format = BLOGGER_TIMESTAMP_FORMAT)
        {
            timestamp_format() = new_format;
        }

        static void SetEnding(InString ending = BLOGGER_TERMINATE_WITH)
        {
            end() = ending;
        }
    private:
        template<typename T>
        static void find_and_replace(String& in, InString what, T&& with)
        {
            auto pos = in.find(what);
            if (pos == String::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, to_string(with).c_str());
        }

        static void find_and_replace_timestamp(String& in, InString what, std::tm* time)
        {
            auto pos = in.find(what);
            if (pos == String::npos) return;

            // If your timestamp is longer than this
            // then you're doing something wrong...
            constexpr size_t ts_size = 128;

            char_t timestamp[ts_size];

            auto written = BLOGGER_TIME_TO_STRING(timestamp, ts_size, timestamp_format().c_str(), time);

            in.erase(pos, timestamp_format().size());

            if (!written) return;

            in.insert(pos, timestamp);
        }

        static void find_and_replace_level(String& in, InString what, level lvl)
        {
            auto pos = in.find(what);
            if (pos == String::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, level_to_string(lvl));
        }

        template<typename T>
        static void format_one(String& out_formatted, uint16_t& index, T&& arg)
        {
            String stringed_arg = to_string(arg);

            String pos_arg;
            pos_arg += BLOGGER_ARG_OPENING;
            pos_arg += to_string(index);
            pos_arg += BLOGGER_ARG_CLOSING;

            auto arg_offset = out_formatted.find(pos_arg);
            if (arg_offset == String::npos)
            {
                arg_offset = out_formatted.find(BLOGGER_ARG_FULL);
                pos_arg = BLOGGER_ARG_FULL;
            }
            if (arg_offset == String::npos)
                return;

            index++;

            out_formatted.erase(arg_offset, pos_arg.size());
            out_formatted.insert(arg_offset, stringed_arg.c_str());
        }

        static String& overflow_postfix()
        {
            static String overflow_postfix = BLOGGER_OVERFLOW_POSTFIX;
            return overflow_postfix;
        }

        static uint64_t& max_length()
        {
            static uint64_t length = BLOGGER_INFINITE;
            return length;
        }

        static String& timestamp_format()
        {
            static String timestamp_format = BLOGGER_TIMESTAMP_FORMAT;
            return timestamp_format;
        }

        static String& end()
        {
            static String end = BLOGGER_TERMINATE_WITH;
            return end;
        }
    };
}
