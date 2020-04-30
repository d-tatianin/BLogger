#pragma once

#include "blogger/formatter.h"
#include "blogger/log_levels.h"

namespace bl {

    struct log_message
    {
    private:
        string  m_formatted_msg;
        string  m_final_pattern;
        std::tm m_time_point;
        level   m_level;
    public:
        log_message(
            string&& formatted_msg,
            string&& ptrn,
            std::tm tp,
            level lvl
        ) : m_formatted_msg(std::move(formatted_msg)),
            m_final_pattern(std::move(ptrn)),
            m_time_point(tp),
            m_level(lvl)
        {
        }

        void finalize_format()
        {
            formatter::merge_pattern(
                m_formatted_msg,
                m_final_pattern,
                time_point_ptr(),
                m_level
            );
        }

        const char_t* data()
        {
            return m_final_pattern.data();
        }

        size_t size()
        {
            return m_final_pattern.size();
        }

        level log_level()
        {
            return m_level;
        }
    private:
        std::tm* time_point_ptr()
        {
            return &m_time_point;
        }
    };
}
