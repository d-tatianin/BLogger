#pragma once

#include <memory>
#include <array>
#include <string>
#include <sstream>
#include <vector>
#include <mutex>

#include "BLogger/OS/Functions.h"
#include "BLogger/LogLevels.h"

#ifdef BLOGGER_UNICODE_MODE
    #ifdef _WIN32
        #define INTERNAL_FORMAT_ARG BLOGGER_MAKE_UNICODE('s')
        #define FULL_INTERNAL_FORMAT_ARG BLOGGER_MAKE_UNICODE("%s")
    #elif defined(__linux__)
        #define INTERNAL_FORMAT_ARG BLOGGER_MAKE_UNICODE('S')
        #define FULL_INTERNAL_FORMAT_ARG BLOGGER_MAKE_UNICODE("%S")
    #endif
#else
    #define INTERNAL_FORMAT_ARG 's'
    #define FULL_INTERNAL_FORMAT_ARG "%s"
#endif

namespace BLogger
{
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
        blogger_basic_pattern()
            : m_Buffer(BLOGGER_BUFFER_SIZE),
            m_HasTimestamp(false),
            m_HasMsg(false),
            m_MsgFirst(false),
            m_HasLvl(false),
            m_TimeOffset(0),
            m_TimeSize(0)
        {
        }

        void init()
        {
            m_Buffer.resize(BLOGGER_BUFFER_SIZE);

            m_HasTimestamp = false;
            m_HasMsg       = false;
            m_MsgFirst     = false;
            m_HasLvl       = false;
            m_TimeOffset   = 0;
            m_TimeSize     = 0;

            memset(m_Buffer.data(), 0, m_Buffer.size());
        }

        bl_char* ts_begin()
        {
            return m_Buffer.data() + m_TimeOffset;
        }

        size_t ts_size()
        {
            return m_TimeSize;
        }

        bl_char last_ts_char()
        {
            return m_Buffer[ptr_to_index(ts_begin() + m_TimeSize)];
        }

        bl_char* data()
        {
            return m_Buffer.data();
        }

        size_t size()
        {
            return m_Buffer.size();
        }

        void set_memorized(bl_char symbol)
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

        size_t zero_term()
        {
            auto loc = std::find(m_Buffer.begin(), m_Buffer.end(), BLOGGER_MAKE_UNICODE('\0'));

            return loc != m_Buffer.end() ? ptr_to_index(&*loc) : m_Buffer.size();
        }

        bool set_pattern(
            BLoggerInString pattern,
            BLoggerInString tag
        )
        {
            if (pattern.empty())
                return true;

            #define BLOGGER_TS_PATTERN  BLOGGER_MAKE_UNICODE("{ts}")
            #define BLOGGER_TAG_PATTERN BLOGGER_MAKE_UNICODE("{tag}")
            #define BLOGGER_LVL_PATTERN BLOGGER_MAKE_UNICODE("{lvl}")
            #define BLOGGER_MSG_PATTERN BLOGGER_MAKE_UNICODE("{msg}")

            MEMORY_COPY(m_Buffer.data(), m_Buffer.size(), pattern.data(), pattern.size());

            auto ts_offset  = pattern.find(BLOGGER_TS_PATTERN);
            auto tag_offset = pattern.find(BLOGGER_TAG_PATTERN);
            auto lvl_offset = pattern.find(BLOGGER_LVL_PATTERN);
            auto msg_offset = pattern.find(BLOGGER_MSG_PATTERN);

            bool first_arg = true;

            if (msg_offset != BLoggerString::npos && lvl_offset != BLoggerString::npos)
            {
                m_MsgFirst = msg_offset < lvl_offset;
                m_HasMsg = true;
            }
            else if (msg_offset != BLoggerString::npos)
            {
                m_MsgFirst = true;
                m_HasMsg = true;
            }

            for (size_t i = 0; i < pattern.size(); i++)
            {
                if (ts_offset != BLoggerString::npos && ts_offset == i)
                {
                    m_HasTimestamp = true;

                    m_TimeOffset = first_arg ?
                       ts_offset :
                       new_offset(BLOGGER_TS_PATTERN);

                    m_TimeSize = STRING_LENGTH(BLOGGER_TIMESTAMP);

                    set_arg(
                        first_arg ? 
                        ts_offset : 
                        new_offset(BLOGGER_TS_PATTERN),
                        BLOGGER_TIMESTAMP,
                        BLOGGER_TS_PATTERN
                    );

                    first_arg = false;
                }

                if (tag_offset != BLoggerString::npos && tag_offset == i)
                {
                    set_arg(
                        first_arg ?
                        tag_offset :
                        new_offset(BLOGGER_TAG_PATTERN),
                        tag.data(),
                        BLOGGER_TAG_PATTERN
                    );

                    first_arg = false;
                }

                if (lvl_offset != BLoggerString::npos && lvl_offset == i)
                {
                    m_HasLvl = true;

                    set_arg(
                        first_arg ?
                        lvl_offset :
                        new_offset(BLOGGER_LVL_PATTERN),
                        FULL_INTERNAL_FORMAT_ARG, 
                        BLOGGER_LVL_PATTERN
                    );

                    first_arg = false;
                }

                if (msg_offset != BLoggerString::npos && msg_offset == i)
                {
                    set_arg(
                        first_arg ?
                        msg_offset :
                        new_offset(BLOGGER_MSG_PATTERN),
                        FULL_INTERNAL_FORMAT_ARG,
                        BLOGGER_MSG_PATTERN
                    );

                    first_arg = false;
                }
            }

            return true;
        }

        blogger_basic_pattern<bufferT>& operator=(const blogger_basic_pattern<bufferT>& other)
        {
            this->m_Buffer = other.m_Buffer;
            this->m_HasTimestamp = other.m_HasTimestamp;
            this->m_HasMsg = other.m_HasMsg;
            this->m_MsgFirst = other.m_MsgFirst;
            this->m_HasLvl = other.m_HasLvl;
            this->m_TimeOffset = other.m_TimeOffset;
            this->m_TimeSize = other.m_TimeSize;

            return *this;
        }

        private:
            size_t ptr_to_index(bl_char* p)
            {
                return (p - m_Buffer.data());
            }

            size_t new_offset(const bl_char* arg)
            {
                auto index = std::search(
                    m_Buffer.begin(),
                    m_Buffer.end(),
                    arg,
                    arg +
                    STRING_LENGTH(arg)
                );

                return ptr_to_index(&*index);
            }

            void set_arg(size_t offset, const bl_char* pattern, const bl_char* arg)
            {
                m_Buffer[offset] = BLOGGER_MAKE_UNICODE('%');
                m_Buffer[offset + 1] = INTERNAL_FORMAT_ARG;

                int32_t extra_size = static_cast<int32_t>(STRING_LENGTH(arg) - 2);

                bl_char* arg_end = (&m_Buffer[offset]) + 2;

                if (extra_size > 0)
                {
                    MEMORY_MOVE(
                        arg_end,
                        m_Buffer.size() -
                        ptr_to_index(arg_end),
                        arg_end +
                        extra_size,
                        m_Buffer.size() -
                        ptr_to_index(arg_end + extra_size)
                    );
                }

                bl_char copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, m_Buffer.data(), m_Buffer.size());

                auto size = m_Buffer.size() -
                    ptr_to_index(arg_end);

                FORMAT_STRING(
                    &m_Buffer[offset],
                    size,
                    copy + offset,
                    pattern
                );
            }
    };

    typedef blogger_basic_pattern<BLoggerBuffer>
        BLoggerPattern;
    typedef std::shared_ptr<BLoggerPattern>
        BLoggerSharedPattern;

    template<typename bufferT>
    class blogger_basic_formatter
    {
    protected:
        bufferT    m_Buffer;
        bl_char*   m_Cursor;
        size_t     m_Occupied;
        size_t     m_ArgCount;
    public:
        blogger_basic_formatter()
            : m_Buffer(BLOGGER_BUFFER_SIZE),
            m_Cursor(m_Buffer.data()),
            m_Occupied(0),
            m_ArgCount(0)
        {
        }

        template<typename T>
        void handle_pack(T&& arg)
        {
            BLoggerStringStream ss;

            *this << *static_cast<BLoggerStringStream*>(&(ss << std::forward<T&&>(arg)));
        }

        int32_t remaining()
        {
            return static_cast<int32_t>(m_Buffer.size()) -
                   static_cast<int32_t>(m_Occupied);
        }

        bl_char* data()
        {
            return m_Buffer.data();
        }

        bl_char* cursor()
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
            m_Buffer.resize(m_Occupied);
            return std::move(m_Buffer);
        }

        void reset_buffer()
        {
            m_Buffer.resize(BLOGGER_BUFFER_SIZE);
            m_Occupied = 0;
            m_ArgCount = 0;
            m_Cursor = m_Buffer.data();
            memset(m_Buffer.data(), 0, m_Buffer.size());
        }

        void write_to(const bl_char* data, size_t size)
        {
            if ((m_Buffer.size() - m_Occupied) >= size)
            {
                MEMORY_COPY(m_Cursor, remaining(), data, size);
                m_Cursor += size;
                m_Occupied += size;
            }
        }

        void process_message(const bl_char* msg, size_t size)
        {
            write_to(msg, size);
        }

        #pragma warning(push)
        #pragma warning(disable:6054) // String might not be zero-terminated (we don't care)

        static void merge_pattern(
            bl_string& formatted_msg,
            BLoggerSharedPattern global_pattern,
            std::tm* time_ptr, 
            level lvl
        )
        {
            BLoggerPattern ptrn;
            ptrn = *global_pattern;

            size_t message_size = formatted_msg.size() + 1;
            bl_char* message; STACK_ALLOC(message_size, message);
            message[message_size - 1] = '\0';
            MEMORY_COPY(message, message_size, formatted_msg.data(), formatted_msg.size());

            if (ptrn.timestamp())
            {
                bl_char memorized = ptrn.last_ts_char();
                auto intended_size = TIME_TO_STRING(ptrn.ts_begin(), ptrn.size(), BLOGGER_TIMESTAMP, time_ptr);
                ptrn.set_memorized(memorized);

                if (!ptrn.lvl() && !ptrn.msg())
                {
                    formatted_msg.resize(ptrn.zero_term());
                    MEMORY_COPY(formatted_msg.data(), formatted_msg.size(), ptrn.data(), formatted_msg.size());
                }
            }

            if (ptrn.lvl() && ptrn.msg())
            {
                bl_char copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = FORMAT_STRING(
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

                intended_size > BLOGGER_BUFFER_SIZE ?
                    formatted_msg.resize(BLOGGER_BUFFER_SIZE):
                    formatted_msg.resize(intended_size);

                MEMORY_COPY(formatted_msg.data(), formatted_msg.size(), ptrn.data(), ptrn.zero_term());
            }
            else if(ptrn.lvl())
            {
                bl_char copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = FORMAT_STRING(
                    ptrn.data(),
                    ptrn.size(),
                    copy,
                    LevelToString(lvl)
                );

                intended_size > BLOGGER_BUFFER_SIZE ?
                    formatted_msg.resize(BLOGGER_BUFFER_SIZE):
                    formatted_msg.resize(intended_size);

                MEMORY_COPY(formatted_msg.data(), formatted_msg.size(), ptrn.data(), formatted_msg.size());
            }
            else if (ptrn.msg())
            {
                bl_char copy[BLOGGER_BUFFER_SIZE];
                MEMORY_COPY(copy, BLOGGER_BUFFER_SIZE, ptrn.data(), ptrn.size());

                auto intended_size = FORMAT_STRING(
                    ptrn.data(),
                    ptrn.size(),
                    copy,
                    message
                );

                intended_size > BLOGGER_BUFFER_SIZE ?
                    formatted_msg.resize(BLOGGER_BUFFER_SIZE):
                    formatted_msg.resize(intended_size);

                MEMORY_COPY(formatted_msg.data(), formatted_msg.size(), ptrn.data(), formatted_msg.size());
            }
            else if (!ptrn.lvl() && !ptrn.msg() && !ptrn.timestamp())
            {
                formatted_msg.resize(ptrn.zero_term());
                MEMORY_COPY(formatted_msg.data(), formatted_msg.size(), ptrn.data(), formatted_msg.size());
            }

            if (formatted_msg.size() < BLOGGER_BUFFER_SIZE)
                formatted_msg.emplace_back('\n');
            else
                formatted_msg[formatted_msg.size() - 1] = '\n';
        }
    private:
        void operator<<(BLoggerStringStream& ss)
        {
            BLoggerString pattern = BLOGGER_MAKE_UNICODE("{");
            pattern += TO_STRING(m_ArgCount++);
            pattern += BLOGGER_MAKE_UNICODE("}");
            bl_char* cursor = get_pos_arg(pattern);

            if (cursor)
            {
                cursor[0] = '%';
                cursor[1] = INTERNAL_FORMAT_ARG;

                size_t offset = cursor - m_Buffer.data() + 2;
                bl_char* begin = m_Buffer.data() + offset + pattern.size() - 2;
                size_t end = m_Buffer.size() - offset + pattern.size() - 2;

                MEMORY_MOVE(m_Buffer.data() + offset, m_Buffer.size(), begin, end);
            }
            else
            {
                cursor = get_next_arg();

                if (!cursor) return;

                *cursor = '%';
                *(cursor + 1) = INTERNAL_FORMAT_ARG;
            }

            bl_char format[BLOGGER_BUFFER_SIZE];
            MEMORY_COPY(format, BLOGGER_BUFFER_SIZE, m_Buffer.data(), m_Buffer.size());

            auto intended_size =
                FORMAT_STRING(
                    m_Buffer.data(),
                    m_Buffer.size(),
                    format,
                    ss.str().c_str()
                );

            // if FORMAT_STRING returned more bytes than we have
            // that means that our buffer is full
            // so we just set the capacity to zero
            intended_size > m_Buffer.size() ?
                m_Occupied = m_Buffer.size() :
                m_Occupied = intended_size;
            m_Cursor = m_Buffer.data() + m_Occupied;
        }
        #pragma warning(pop) // 6054 ^

        void write_to_enclosed(const bl_char* data, size_t size, bl_char opening = '[', bl_char closing = ']')
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

        bl_char* get_next_arg()
        {
            auto index = std::search(
                m_Buffer.begin(),
                m_Buffer.end(),
                BLOGGER_ARG_PATTERN,
                BLOGGER_ARG_PATTERN +
                STRING_LENGTH(BLOGGER_ARG_PATTERN)
            );

            return (index != m_Buffer.end() ? &(*index) : nullptr);
        }

        bl_char* get_pos_arg(BLoggerInString pos)
        {
            auto cursor = std::search(
                m_Buffer.begin(),
                m_Buffer.end(),
                pos.data(),
                pos.data() +
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
