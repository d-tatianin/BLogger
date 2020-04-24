#pragma once

#include <memory>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#include "blogger/OS/Functions.h"
#include "blogger/LogLevels.h"

namespace bl
{
    class Formatter
    {
        constexpr static auto arg_opening = BLOGGER_WIDEN_IF_NEEDED("{");
        constexpr static auto arg_closing = BLOGGER_WIDEN_IF_NEEDED("}");
        constexpr static auto arg_full    = BLOGGER_WIDEN_IF_NEEDED("{}");

        constexpr static auto timestamp_pattern = BLOGGER_WIDEN_IF_NEEDED("{ts}");
        constexpr static auto tag_pattern       = BLOGGER_WIDEN_IF_NEEDED("{tag}");
        constexpr static auto level_pattern     = BLOGGER_WIDEN_IF_NEEDED("{lvl}");
        constexpr static auto message_pattern   = BLOGGER_WIDEN_IF_NEEDED("{msg}");

        constexpr static auto default_timestamp_format = BLOGGER_WIDEN_IF_NEEDED("%H:%M:%S");

        constexpr static auto default_ending  = BLOGGER_WIDEN_IF_NEEDED("\n");
        constexpr static auto default_postfix = BLOGGER_WIDEN_IF_NEEDED("...");

        friend class Logger;
    public:
        static void CreatePatternFrom(
            String& out_pattern,
            InString tag
        )
        {
            if (out_pattern.empty())
                return;

            find_and_replace(out_pattern, tag_pattern, tag);
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
            find_and_replace(merge_into, message_pattern, formatted_msg);
            find_and_replace(merge_into, timestamp_pattern, timestamp_format().c_str());
            find_and_replace_timestamp(merge_into, timestamp_format().c_str(), time_ptr);
            find_and_replace_level(merge_into, level_pattern, lvl);

            if (max_length() != infinite &&
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
            InString postfix = default_postfix
        )
        {
            max_length() = length;
            overflow_postfix() = postfix;
        }

        // Uses strftime format - https://en.cppreference.com/w/cpp/chrono/c/strftime
        static void SetTimestampFormat(InString new_format = default_timestamp_format)
        {
            timestamp_format() = new_format;
        }

        static void SetEnding(InString ending = default_ending)
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
            in.insert(pos, lvl.to_string());
        }

        template<typename T>
        static void format_one(String& out_formatted, uint16_t& index, T&& arg)
        {
            String stringed_arg = to_string(arg);

            String pos_arg;
            pos_arg += arg_opening;
            pos_arg += to_string(index);
            pos_arg += arg_closing;

            auto arg_offset = out_formatted.find(pos_arg);
            if (arg_offset == String::npos)
            {
                arg_offset = out_formatted.find(arg_full);
                pos_arg = arg_full;
            }
            if (arg_offset == String::npos)
                return;

            index++;

            out_formatted.erase(arg_offset, pos_arg.size());
            out_formatted.insert(arg_offset, stringed_arg.c_str());
        }

        static String& overflow_postfix()
        {
            static String overflow_postfix = default_postfix;
            return overflow_postfix;
        }

        static uint64_t& max_length()
        {
            static uint64_t length = infinite;
            return length;
        }

        static String& timestamp_format()
        {
            static String timestamp_format = default_timestamp_format;
            return timestamp_format;
        }

        static String& end()
        {
            static String end = default_ending;
            return end;
        }
    };
}
