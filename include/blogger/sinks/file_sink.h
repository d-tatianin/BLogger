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
        FILE*         m_File;
        string        m_DirectoryPath;
        string        m_CachedTag;
        size_t        m_BytesPerFile;
        size_t        m_CurrentBytes;
        size_t        m_MaxLogFiles;
        size_t        m_CurrentLogFiles;
        bool          m_RotateLogs;
        std::mutex    m_FileAccess;
    public:
        file_sink(
            in_string directoryPath,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true
        ) : m_File(nullptr),
            m_DirectoryPath(directoryPath),
            m_CachedTag(BLOGGER_WIDEN_IF_NEEDED("logfile")),
            m_BytesPerFile(bytesPerFile),
            m_CurrentBytes(0),
            m_MaxLogFiles(maxLogFiles),
            m_CurrentLogFiles(0),
            m_RotateLogs(rotateLogs),
            m_FileAccess()
        {
            if (m_DirectoryPath.back() != BLOGGER_WIDEN_IF_NEEDED('/'))
                m_DirectoryPath += '/';
        }

        void terminate()
        {
            locker lock(m_FileAccess);

            if (m_File)
            {
                fclose(m_File);
                m_File = nullptr;
            }
        }

        bool ok()
        {
            return static_cast<bool>(m_File);
        }

        void write(log_message& msg) override
        {
            locker lock(m_FileAccess);

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

            if (m_BytesPerFile != infinite)
            {
                if (BLOGGER_TRUE_SIZE(size) > m_BytesPerFile)
                    return;

                if (m_CurrentBytes + BLOGGER_TRUE_SIZE(size) > m_BytesPerFile)
                    if (!new_log_file()) return;
            }

            m_CurrentBytes += BLOGGER_TRUE_SIZE(size);

            BLOGGER_FILE_WRITE(data, size, m_File);
        }

        void flush() override
        {
            locker lock(m_FileAccess);

            if (m_File)
                fflush(m_File);
        }

        operator bool()
        {
            return ok();
        }

        ~file_sink()
        {
            if (m_File)
                fclose(m_File);
        }

        void set_tag(in_string name) override
        {
            m_CachedTag = name;
            new_log_file();
        }
    private:
        void construct_full_path(
            string& outPath
        )
        {
            outPath += m_DirectoryPath;
            outPath += m_CachedTag;
            outPath += BLOGGER_WIDEN_IF_NEEDED('-');
            outPath += BLOGGER_TO_STRING(m_CurrentLogFiles);
            outPath += BLOGGER_WIDEN_IF_NEEDED(".log");
        }

        bool new_log_file()
        {
            if (m_CurrentLogFiles == m_MaxLogFiles)
            {
                if (!m_RotateLogs)
                    return false;
                else
                {
                    m_CurrentLogFiles = 1;
                    m_CurrentBytes = 0;
                }
            }
            else
            {
                m_CurrentBytes = 0;
                ++m_CurrentLogFiles;
            }

            if (m_File)
            {
                fclose(m_File);
                m_File = nullptr;
            }

            string fullPath;
            construct_full_path(fullPath);

            BLOGGER_OPEN_FILE(m_File, fullPath);

            return m_File;
        }
    };
}
