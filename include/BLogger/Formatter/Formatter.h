#pragma once

#include "BLogger/LogLevels.h"
#include "FormatUtilities.h"
#include "BLogger/OS/Functions.h"

#define BLOGGER_BUFFER_SIZE 128
#define BLOGGER_TIMESTAMP "%H:%M:%S"
#define BLOGGER_ARG_PATTERN "{}"

namespace BLogger
{
    typedef char charT;

    template<size_t size>
    using internal_buffer = std::array<charT, size>;

    typedef internal_buffer<BLOGGER_BUFFER_SIZE>
        BLoggerBuffer;

    template<typename bufferT>
    class blogger_basic_pattern
    {
    private:
        bufferT m_Buffer;

        bool m_HasTimestamp;
        bool m_HasMsg;
        bool m_MsgFirst;
        bool m_HasLvl;

        size_t m_TimeOffset;
        size_t m_TimeSize;
    public:
        void init()
        {
            m_HasTimestamp      = false;
            bool m_HasMsg       = false;
            bool m_MsgFirst     = false;
            bool m_HasLvl       = false;
            size_t m_TimeOffset = 0;
            size_t m_TimeSize   = 0;

            memset(m_Buffer.data(), 0, m_Buffer.size());
        }

        charT* ts_begin()
        {
            return m_Buffer.data() + m_TimeOffset;
        }

        size_t ts_size()
        {
            return m_TimeSize;
        }

        charT last_ts_char()
        {
            return m_Buffer[ptr_to_index(ts_begin() + m_TimeSize)];
        }

        charT* data()
        {
            return m_Buffer.data();
        }

        size_t size()
        {
            return m_Buffer.size();
        }

        void set_memorized(charT symbol)
        {
            m_Buffer[ptr_to_index(ts_begin() + m_TimeSize)] = symbol;
        }

        bool timestamp()
        {
            return m_HasTimestamp;
        }

        bool lvl()
        {
            return m_HasLvl;
        }

        bool msg()
        {
            return m_HasMsg;
        }

        bool msg_first()
        {
            return m_MsgFirst;
        }

        bool set_pattern(
            const std::string& pattern,
            const std::string& tag
        )
        {
            #define BLOGGER_TS_PATTERN  "{ts}"
            #define BLOGGER_TAG_PATTERN "{tag}"
            #define BLOGGER_LVL_PATTERN "{lvl}"
            #define BLOGGER_MSG_PATTERN "{msg}"

            MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), pattern.c_str(), pattern.size());

            auto ts_offset  = pattern.find(BLOGGER_TS_PATTERN);
            auto tag_offset = pattern.find(BLOGGER_TAG_PATTERN);
            auto lvl_offset = pattern.find(BLOGGER_LVL_PATTERN);
            auto msg_offset = pattern.find(BLOGGER_MSG_PATTERN);

            bool first_arg = true;

            if (msg_offset != std::string::npos && lvl_offset != std::string::npos)
            {
                m_MsgFirst = msg_offset < lvl_offset;
                m_HasMsg = true;
            }

            for (size_t i = 0; i < pattern.size(); i++)
            {
                if (ts_offset != std::string::npos && ts_offset == i)
                {
                    m_HasTimestamp = true;

                    m_TimeOffset = first_arg ?
                       ts_offset :
                       new_offset(BLOGGER_TS_PATTERN);

                    m_TimeSize = strlen(BLOGGER_TIMESTAMP);

                    set_arg(
                        first_arg ? 
                        ts_offset : 
                        new_offset(BLOGGER_TS_PATTERN),
                        BLOGGER_TIMESTAMP,
                        BLOGGER_TS_PATTERN
                    );

                    first_arg = false;
                }

                if (tag_offset != std::string::npos && tag_offset == i)
                {
                    set_arg(
                        first_arg ?
                        tag_offset :
                        new_offset(BLOGGER_TAG_PATTERN),
                        tag.c_str(),
                        BLOGGER_TAG_PATTERN
                    );

                    first_arg = false;
                }

                if (lvl_offset != std::string::npos && lvl_offset == i)
                {
                    m_HasLvl = true;

                    set_arg(
                        first_arg ?
                        lvl_offset :
                        new_offset(BLOGGER_LVL_PATTERN),
                        "%s", 
                        BLOGGER_LVL_PATTERN
                    );

                    first_arg = false;
                }

                if (msg_offset != std::string::npos && msg_offset == i)
                {
                    set_arg(
                        first_arg ?
                        msg_offset :
                        new_offset(BLOGGER_MSG_PATTERN),
                        "%s",
                        BLOGGER_MSG_PATTERN
                    );

                    first_arg = false;
                }
            }

            return true;
        }
        private:
            size_t ptr_to_index(charT* p)
            {
                return (p - m_Buffer.data());
            }

            size_t new_offset(charT* arg)
            {
                auto index = std::search(
                    m_Buffer.begin(),
                    m_Buffer.end(),
                    arg,
                    arg +
                    std::strlen(arg)
                );

                return ptr_to_index(&*index);
            }

            void set_arg(size_t offset, const charT* pattern, const charT* arg)
            {
                m_Buffer[offset] = '%';
                m_Buffer[offset + 1] = 's';

                int32_t extra_size = std::strlen(arg) - 2;

                charT* arg_end = (&m_Buffer[offset]) + 2;

                if (extra_size > 0)
                {
                    MEMORY_MOVE(arg_end, m_Buffer.size() - ptr_to_index(arg_end), arg_end + extra_size, m_Buffer.size() - ptr_to_index(arg_end + extra_size));
                }

                charT copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, m_Buffer.data(), m_Buffer.size());

                auto size = m_Buffer.size() -
                    ptr_to_index(arg_end);

                snprintf(
                    &m_Buffer[offset],
                    size,
                    copy + offset,
                    pattern
                );
            }
    };

    typedef blogger_basic_pattern<BLoggerBuffer>
        BLoggerPattern;

    template<typename bufferT>
    class blogger_basic_formatter
    {
    protected:
        bufferT    m_Buffer;
        charT*     m_Cursor;
        size_t     m_Occupied;
        size_t     m_ArgCount;
    public:
        blogger_basic_formatter()
            : m_Buffer(),
            m_Cursor(m_Buffer.data()),
            m_Occupied(0),
            m_ArgCount(0)
        {
        }

        template<typename T>
        void handle_pack(T&& arg)
        {
            std::stringstream ss;

            *this << *static_cast<std::stringstream*>(&(ss << std::forward<T&&>(arg)));
        }

        int32_t remaining()
        {
            return static_cast<int32_t>(m_Buffer.size()) -
                   static_cast<int32_t>(m_Occupied);
        }

        charT* data()
        {
            return m_Buffer.data();
        }

        charT* cursor()
        {
            return m_Cursor;
        }

        size_t size()
        {
            return m_Occupied;
        }

        void advance_cursor_by(size_t count)
        {
            m_Cursor += count;
            m_Occupied += count;
        }

        BLoggerBuffer& get_buffer()
        {
            return m_Buffer;
        }

        BLoggerBuffer&& release_buffer()
        {
            return std::move(m_Buffer);
        }

        void reset_buffer()
        {
            m_Occupied = 0;
            m_ArgCount = 0;
            m_Cursor = m_Buffer.data();
            memset(m_Buffer.data(), 0, m_Buffer.size());
        }

        void write_to(const charT* data, size_t size)
        {
            if ((m_Buffer.size() - m_Occupied) >= size)
            {
                MEMORY_COPY(m_Cursor, remaining(), data, size);
                m_Cursor += size;
                m_Occupied += size;
            }
        }

        void process_message(const charT* msg, size_t size)
        {
            write_to(msg, size);
        }

        void merge_pattern(
            BLoggerPattern& ptrn, 
            std::tm* time_ptr, 
            level lvl
        )
        {
            charT* message = static_cast<charT*>(STACK_ALLOC(m_Occupied + 1));
            message[m_Occupied] = '\0';
            MEMORY_COPY(message, m_Occupied, m_Buffer.data(), m_Occupied);

            if (ptrn.timestamp())
            {
                charT memorized = ptrn.last_ts_char();
                auto writtern = strftime(ptrn.ts_begin(), ptrn.size(), BLOGGER_TIMESTAMP, time_ptr);
                ptrn.set_memorized(memorized);

                if(!ptrn.lvl() && !ptrn.msg())
                    MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), ptrn.data(), m_Occupied);
            }

            if (ptrn.lvl() && ptrn.msg())
            {
                charT copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = snprintf(
                    ptrn.data(),
                    ptrn.size(),
                    copy,
                    (ptrn.msg_first() ?
                    message :
                    LevelToString(lvl)),
                    (ptrn.msg_first() ?
                    LevelToString(lvl) :
                    message)
                );

                intended_size > m_Buffer.size() ?
                    m_Occupied = m_Buffer.size() :
                    m_Occupied = intended_size;

                MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), ptrn.data(), m_Occupied);
            }
            else if(ptrn.lvl())
            {
                charT copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = snprintf(
                    ptrn.data(),
                    ptrn.size(),
                    copy,
                    LevelToString(lvl)
                );

                intended_size > m_Buffer.size() ?
                    m_Occupied = m_Buffer.size() :
                    m_Occupied = intended_size;

                MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), copy, m_Occupied);
            }
            else if (ptrn.msg())
            {
                charT copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = snprintf(
                    ptrn.data(),
                    ptrn.size(),
                    copy,
                    message
                );

                intended_size > m_Buffer.size() ?
                    m_Occupied = m_Buffer.size() :
                    m_Occupied = intended_size;

                MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), copy, m_Occupied);
            }

            if (m_Occupied < m_Buffer.size())
                m_Buffer[m_Occupied++] = '\n';
            else
                m_Buffer[m_Occupied - 1] = '\n';
        }
    private:
        void operator<<(std::stringstream& ss)
        {
            std::string pattern = "{";
            pattern += std::to_string(m_ArgCount++);
            pattern += "}";
            charT* cursor = get_pos_arg(pattern);

            if (cursor)
            {
                cursor[0] = '%';
                cursor[1] = 's';

                size_t offset = cursor - m_Buffer.data() + 2;
                charT* begin = m_Buffer.data() + offset + pattern.size() - 2;
                size_t end = m_Buffer.size() - offset + pattern.size() - 2;

                MEMORY_MOVE(m_Buffer.data() + offset, m_Buffer.size(), begin, end);
            }
            else
            {
                cursor = get_next_arg();

                if (!cursor) return;

                *cursor = '%';
                *(cursor + 1) = 's';
            }

            charT format[BLOGGER_BUFFER_SIZE];
            MEMORY_COPY(format, BLOGGER_BUFFER_SIZE, m_Buffer.data(), m_Buffer.size());

            auto intended_size =
                snprintf(
                    m_Buffer.data(),
                    m_Buffer.size(),
                    format,
                    ss.str().c_str()
                );

            // if snprintf returned more bytes than we have
            // that means that our buffer is full
            // so we just set the capacity to zero
            intended_size > m_Buffer.size() ?
                m_Occupied = m_Buffer.size() :
                m_Occupied = intended_size;
            m_Cursor = m_Buffer.data() + m_Occupied;
        }

        void write_to_enclosed(const charT* data, size_t size, charT opening = '[', charT closing = ']')
        {
            if ((m_Buffer.size() - m_Occupied) >= size + 2)
            {
                *(m_Cursor++) = opening;
                ++m_Occupied;
                MEMORY_COPY(m_Cursor, remaining(), data, size);
                m_Occupied += size;
                m_Cursor += size;
                *(m_Cursor++) = closing;
                ++m_Occupied;
            }
        }

        charT* get_next_arg()
        {
            auto index = std::search(
                m_Buffer.begin(),
                m_Buffer.end(),
                BLOGGER_ARG_PATTERN,
                BLOGGER_ARG_PATTERN +
                std::strlen(BLOGGER_ARG_PATTERN)
            );

            return (index != m_Buffer.end() ? &(*index) : nullptr);
        }

        charT* get_pos_arg(const std::string& pos)
        {
            auto cursor = std::search(
                m_Buffer.begin(),
                m_Buffer.end(),
                pos.c_str(),
                pos.c_str() +
                pos.size()
            );

            return (cursor != m_Buffer.end() ? &(*cursor) : nullptr);
        }
    };

    typedef blogger_basic_formatter<BLoggerBuffer>
        BLoggerFormatter;
}

#undef BLOGGER_BUFFER_SIZE
#undef BLOGGER_ARG_PATTERN
