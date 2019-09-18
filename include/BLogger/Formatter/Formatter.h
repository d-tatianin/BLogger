#pragma once

#include "BLogger/LogLevels.h"
#include "FormatUtilities.h"
#include "BLogger/OS/Functions.h"

#define BLOGGER_BUFFER_SIZE 128
#define BLOGGER_TS_PATTERN "%H:%M:%S"
#define BLOGGER_ARG_PATTERN "{}"

namespace BLogger
{
    typedef char charT;

    template<size_t size>
    using internal_buffer = std::array<charT, size>;

    typedef internal_buffer<BLOGGER_BUFFER_SIZE>
        BLoggerBuffer;

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

        void init_timestamp()
        {
            write_to_enclosed(
                BLOGGER_TS_PATTERN,
                std::strlen(BLOGGER_TS_PATTERN)
            );
        }

        template<typename T>
        void handle_pack(T&& arg)
        {
            std::stringstream ss;

            *this << *static_cast<std::stringstream*>(&(ss << std::forward<T&&>(arg)));
        }

        void add_space()
        {
            if ((m_Buffer.size() - m_Occupied) >= 1)
            {
                *(m_Cursor++) = ' ';
                m_Occupied += 1;
            }
        }

        void append_level(level lvl)
        {
            write_to_enclosed(
                LevelToString(lvl),
                std::strlen(LevelToString(lvl))
            );
        }

        void append_tag(const std::string& tag)
        {
            write_to_enclosed(
                tag.c_str(),
                tag.size()
            );
        }

        void newline()
        {
            if (!remaining())
            {
                --m_Cursor;
                *(m_Cursor++) = '\n';
            }
            else
            {
                m_Occupied += 1;
                *(m_Cursor++) = '\n';
            }
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
