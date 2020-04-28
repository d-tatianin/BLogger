#pragma once

#include <memory>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#include "blogger/os/functions.h"
#include "blogger/log_levels.h"

namespace bl
{
    class formatter
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

        friend class logger;
    public:
        static void create_pattern_from(
            string& out_pattern,
            in_string tag
        )
        {
            if (out_pattern.empty())
                return;

            find_and_replace(out_pattern, tag_pattern, tag);
        }

        template<typename... Args>
        static string format(string pattern, Args&& ... args)
        {
            uint16_t index = 0;
            BLOGGER_FOR_EACH_DO(format_one, args, pattern, index);
            return pattern;
        }

        static void merge_pattern(
            string& formatted_msg,
            string& merge_into,
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

            merge_into += ending();
        }

        static void cut_if_exceeds(
            size_t length,
            in_string postfix = default_postfix
        )
        {
            max_length() = length;
            overflow_postfix() = postfix;
        }

        // Uses strftime format - https://en.cppreference.com/w/cpp/chrono/c/strftime
        static void set_timestamp_format(in_string new_format = default_timestamp_format)
        {
            timestamp_format() = new_format;
        }

        static void set_ending(in_string ending = default_ending)
        {
            bl::formatter::ending() = ending;
        }
    private:
        template<typename T>
        static void find_and_replace(string& in, in_string what, T&& with)
        {
            auto pos = in.find(what);
            if (pos == string::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, to_string(with).c_str());
        }

        static void find_and_replace_timestamp(string& in, in_string what, std::tm* time)
        {
            auto pos = in.find(what);
            if (pos == string::npos) return;

            // If your timestamp is longer than this
            // then you're doing something wrong...
            constexpr size_t ts_size = 128;

            char_t timestamp[ts_size];

            auto written = BLOGGER_TIME_TO_STRING(timestamp, ts_size, timestamp_format().c_str(), time);

            in.erase(pos, timestamp_format().size());

            if (!written) return;

            in.insert(pos, timestamp);
        }

        static void find_and_replace_level(string& in, in_string what, level lvl)
        {
            auto pos = in.find(what);
            if (pos == string::npos) return;

            in.erase(pos, what.size());
            in.insert(pos, lvl.to_string());
        }

        template<typename T>
        static void format_one(string& out_formatted, uint16_t& index, T&& arg)
        {
            string stringed_arg = to_string(arg);

            string pos_arg;
            pos_arg += arg_opening;
            pos_arg += to_string(index);
            pos_arg += arg_closing;

            auto arg_offset = out_formatted.find(pos_arg);
            if (arg_offset == string::npos)
            {
                arg_offset = out_formatted.find(arg_full);
                pos_arg = arg_full;
            }
            if (arg_offset == string::npos)
                return;

            index++;

            out_formatted.erase(arg_offset, pos_arg.size());
            out_formatted.insert(arg_offset, stringed_arg.c_str());
        }

        static string& overflow_postfix()
        {
            static string s_overflow_postfix = default_postfix;
            return s_overflow_postfix;
        }

        static size_t& max_length()
        {
            static size_t s_length = infinite;
            return s_length;
        }

        static string& timestamp_format()
        {
            static string s_timestamp_format = default_timestamp_format;
            return s_timestamp_format;
        }

        static string& ending()
        {
            static string s_end = default_ending;
            return s_end;
        }
    };
}
