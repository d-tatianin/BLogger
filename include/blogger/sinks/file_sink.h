#pragma once

#include <stdio.h>
#include <string>
#include <mutex>

#include "blogger/sinks/sink.h"
#include "blogger/os/functions.h"

namespace bl {

    class file_sink : public sink
    {
    private:
        FILE*      m_file;
        string     m_directory_path;
        string     m_cached_tag;
        size_t     m_bytes_per_file;
        size_t     m_current_bytes;
        size_t     m_max_log_files;
        size_t     m_current_log_files;
        bool       m_rotate_logs;
        std::mutex m_file_access;
    public:
        file_sink(
            in_string directory_path,
            size_t bytes_per_file,
            size_t max_log_files,
            bool rotate_logs = true
        ) : m_file(nullptr),
            m_directory_path(directory_path),
            m_cached_tag(BLOGGER_WIDEN_IF_NEEDED("logfile")),
            m_bytes_per_file(bytes_per_file),
            m_current_bytes(0),
            m_max_log_files(max_log_files),
            m_current_log_files(0),
            m_rotate_logs(rotate_logs),
            m_file_access()
        {
            if (m_directory_path.back() != BLOGGER_WIDEN_IF_NEEDED('/'))
                m_directory_path += '/';
        }

        void terminate()
        {
            locker lock(m_file_access);

            if (m_file)
            {
                fclose(m_file);
                m_file = nullptr;
            }
        }

        bool ok()
        {
            return static_cast<bool>(m_file);
        }

        void write(log_message& msg) override
        {
            locker lock(m_file_access);

            if (!ok())
                return;

        #ifdef BLOGGER_UNICODE_MODE
            size_t byte_limit = msg.size() * 2;
            char* data; BLOGGER_STACK_ALLOC(byte_limit, data);

          #ifdef _WIN32
            auto size =
                WideCharToMultiByte(
                    CP_UTF8, NULL,
                    msg.data(),
                    static_cast<int32_t>(msg.size()),
                    data,
                    static_cast<int32_t>(byte_limit),
                    NULL, NULL
                );
          #else
            auto size = wcstombs(data, msg.data(), byte_limit);
          #endif

            if (!size || size == -1)
                return;
        #else
            auto* data = msg.data();
            auto  size = msg.size();
        #endif

            if (m_bytes_per_file != infinite)
            {
                if (BLOGGER_TRUE_SIZE(size) > m_bytes_per_file)
                    return;

                if (m_current_bytes + BLOGGER_TRUE_SIZE(size) > m_bytes_per_file)
                    if (!new_log_file()) return;
            }

            m_current_bytes += BLOGGER_TRUE_SIZE(size);

            BLOGGER_FILE_WRITE(data, size, m_file);
        }

        void flush() override
        {
            locker lock(m_file_access);

            if (m_file)
                fflush(m_file);
        }

        operator bool()
        {
            return ok();
        }

        ~file_sink()
        {
            if (m_file)
                fclose(m_file);
        }

        void set_tag(in_string name) override
        {
            m_cached_tag = name;
            new_log_file();
        }
    private:
        void construct_full_path(
            string& out_path
        )
        {
            out_path += m_directory_path;
            out_path += m_cached_tag;
            out_path += BLOGGER_WIDEN_IF_NEEDED('-');
            out_path += BLOGGER_TO_STRING(m_current_log_files);
            out_path += BLOGGER_WIDEN_IF_NEEDED(".log");
        }

        bool new_log_file()
        {
            if (m_current_log_files == m_max_log_files)
            {
                if (!m_rotate_logs)
                    return false;
                else
                {
                    m_current_log_files = 1;
                    m_current_bytes = 0;
                }
            }
            else
            {
                m_current_bytes = 0;
                ++m_current_log_files;
            }

            if (m_file)
            {
                fclose(m_file);
                m_file = nullptr;
            }

            string fullPath;
            construct_full_path(fullPath);

            BLOGGER_OPEN_FILE(m_file, fullPath);

            return m_file;
        }
    };
}
