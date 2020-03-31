#pragma once

#include <stdio.h>
#include <string>
#include <mutex>

#include "BLogger/Sinks/Sink.h"
#include "BLogger/OS/Functions.h"

namespace bl {

    class FileSink : public Sink
    {
    private:
        FILE*         m_File;
        String m_DirectoryPath;
        String m_CachedTag;
        size_t        m_BytesPerFile;
        size_t        m_CurrentBytes;
        size_t        m_MaxLogFiles;
        size_t        m_CurrentLogFiles;
        bool          m_RotateLogs;
        std::mutex    m_FileAccess;

        typedef std::lock_guard<std::mutex>
            locker;
    public:
        FileSink(
            InString directoryPath,
            InString loggerTag,
            size_t bytesPerFile,
            size_t maxLogFiles,
            bool rotateLogs = true
        ) : m_File(nullptr),
            m_DirectoryPath(directoryPath),
            m_BytesPerFile(bytesPerFile),
            m_CurrentBytes(0),
            m_MaxLogFiles(0),
            m_CurrentLogFiles(0),
            m_RotateLogs(rotateLogs),
            m_FileAccess()
        {
            m_CachedTag = loggerTag;

            m_BytesPerFile = bytesPerFile;
            m_MaxLogFiles = maxLogFiles;
            m_RotateLogs = rotateLogs;
            m_CurrentBytes = 0;
            m_CurrentLogFiles = 1;

            m_DirectoryPath = directoryPath;
            m_DirectoryPath += '/';

            newLogFile();
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

#ifdef BLOGGER_UNICODE_MODE
    #ifdef _WIN32
        void write(BLoggerLogMessage& msg) override
        {
            locker lock(m_FileAccess);

            if (!ok())
                return;

            size_t byte_limit = msg.size() * 2;
            char* narrow; BLOGGER_STACK_ALLOC(byte_limit, narrow);
            auto mb_size =
                WideCharToMultiByte(
                    CP_UTF8, NULL,
                    msg.data(),
                    static_cast<int32_t>(msg.size()),
                    narrow,
                    static_cast<int32_t>(byte_limit),
                    NULL, NULL
                );

            if (!mb_size) return; // TODO: handle this later

            if (m_BytesPerFile && BLOGGER_TRUE_SIZE(mb_size) > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + BLOGGER_TRUE_SIZE(mb_size)) > m_BytesPerFile)
            {
                if (m_CurrentLogFiles == m_MaxLogFiles)
                {
                    if (!m_RotateLogs)
                        return;
                    else
                    {
                        m_CurrentLogFiles = 1;
                        m_CurrentBytes = 0;
                        newLogFile();
                    }
                }
                else
                {
                    m_CurrentBytes = 0;
                    ++m_CurrentLogFiles;
                    newLogFile();
                }
            }

            m_CurrentBytes += BLOGGER_TRUE_SIZE(mb_size);

            BLOGGER_FILE_WRITE(narrow, mb_size, m_File);
        }
    #elif defined(__linux__)
        void write(BLoggerLogMessage& msg) override
        {
            locker lock(m_FileAccess);

            if (!ok())
                return;

            size_t byte_limit = msg.size() * 2;
            char* narrow; BLOGGER_STACK_ALLOC(byte_limit, narrow);

            auto mb_size = wcstombs(narrow, msg.data(), byte_limit);

            if (mb_size == SIZE_MAX) return; // TODO: handle this later

            if (m_BytesPerFile && BLOGGER_TRUE_SIZE(mb_size) > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + BLOGGER_TRUE_SIZE(mb_size)) > m_BytesPerFile)
            {
                if (m_CurrentLogFiles == m_MaxLogFiles)
                {
                    if (!m_RotateLogs)
                        return;
                    else
                    {
                        m_CurrentLogFiles = 1;
                        m_CurrentBytes = 0;
                        newLogFile();
                    }
                }
                else
                {
                    m_CurrentBytes = 0;
                    ++m_CurrentLogFiles;
                    newLogFile();
                }
            }

            m_CurrentBytes += BLOGGER_TRUE_SIZE(mb_size);

            BLOGGER_FILE_WRITE(narrow, mb_size, m_File);
        }
    #endif
#else
        void write(BLoggerLogMessage& msg) override
        {
            locker lock(m_FileAccess);

            if (!ok())
                return;

            if (m_BytesPerFile && BLOGGER_TRUE_SIZE(msg.size()) > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + BLOGGER_TRUE_SIZE(msg.size())) > m_BytesPerFile)
            {
                if (m_CurrentLogFiles == m_MaxLogFiles)
                {
                    if (!m_RotateLogs)
                        return;
                    else
                    {
                        m_CurrentLogFiles = 1;
                        m_CurrentBytes = 0;
                        newLogFile();
                    }
                }
                else
                {
                    m_CurrentBytes = 0;
                    ++m_CurrentLogFiles;
                    newLogFile();
                }
            }

            m_CurrentBytes += BLOGGER_TRUE_SIZE(msg.size());

            BLOGGER_FILE_WRITE(msg.data(), msg.size(), m_File);
        }
#endif

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

        ~FileSink()
        {
            if (m_File)
                fclose(m_File);
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
