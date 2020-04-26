#pragma once

#include <stdio.h>
#include <string>
#include <mutex>

#include "blogger/Sinks/Sink.h"
#include "blogger/OS/Functions.h"

namespace bl {

    class FileSink : public Sink
    {
    private:
        FILE*         m_File;
        String        m_DirectoryPath;
        String        m_CachedTag;
        size_t        m_BytesPerFile;
        size_t        m_CurrentBytes;
        size_t        m_MaxLogFiles;
        size_t        m_CurrentLogFiles;
        bool          m_RotateLogs;
        std::mutex    m_FileAccess;

        using locker_t = std::lock_guard<std::mutex>;
    public:
        FileSink(
            InString directoryPath,
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
            locker_t lock(m_FileAccess);

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

        void write(LogMessage& msg) override
        {
            locker_t lock(m_FileAccess);

            if (!m_BytesPerFile)
                return;

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

            if (BLOGGER_TRUE_SIZE(size) > m_BytesPerFile)
                return;

            if (m_CurrentBytes + BLOGGER_TRUE_SIZE(size) > m_BytesPerFile)
                newLogFile();

            m_CurrentBytes += BLOGGER_TRUE_SIZE(size);

            BLOGGER_FILE_WRITE(data, size, m_File);
        }

        void flush() override
        {
            locker_t lock(m_FileAccess);

            if (m_File)
                fflush(m_File);
        }

        operator bool()
        {
            return ok();
        }

        ~FileSink()
        {
            if (m_File)
                fclose(m_File);
        }

        void set_name(InString name) override
        {
            m_CachedTag = name;
            newLogFile();
        }
    private:
        void constructFullPath(
            String& outPath
        )
        {
            outPath += m_DirectoryPath;
            outPath += m_CachedTag;
            outPath += BLOGGER_WIDEN_IF_NEEDED('-');
            outPath += BLOGGER_TO_STRING(m_CurrentLogFiles);
            outPath += BLOGGER_WIDEN_IF_NEEDED(".log");
        }

        void newLogFile()
        {
            if (m_CurrentLogFiles == m_MaxLogFiles)
            {
                if (!m_RotateLogs)
                    return;
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

            String fullPath;
            constructFullPath(fullPath);

            BLOGGER_OPEN_FILE(m_File, fullPath);
        }
    };
}
