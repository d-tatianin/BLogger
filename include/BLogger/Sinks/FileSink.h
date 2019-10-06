#pragma once

#include <stdio.h>
#include <string>
#include <mutex>

#include "BLogger/Sinks/BaseSink.h"
#include "BLogger/OS/Functions.h"

namespace BLogger {

    class FileSink : public BaseSink
    {
    private:
        FILE*         m_File;
        BLoggerString m_DirectoryPath;
        BLoggerString m_CachedTag;
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
            BLoggerInString directoryPath,
            BLoggerInString loggerTag,
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

        void set_tag(BLoggerInString tag) override
        {
            locker loc(m_FileAccess);
            m_CachedTag = tag;
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

        void write(BLoggerLogMessage& msg) override
        {
            size_t size = msg.size();

            locker lock(m_FileAccess);

            if (!ok())
                return;

            ++size;

            if (m_BytesPerFile && size > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + size) > m_BytesPerFile)
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

            m_CurrentBytes += size;

            fwrite(msg.data(), 1, size - 1, m_File);
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

        ~FileSink()
        {
            if (m_File)
                fclose(m_File);
        }
    private:
        void constructFullPath(
            BLoggerString& outPath
        )
        {
            outPath += m_DirectoryPath;
            outPath += m_CachedTag;
            outPath += '-';
            outPath += std::to_string(m_CurrentLogFiles);
            outPath += ".log";
        }

        void newLogFile()
        {
            if (m_File)
            {
                fclose(m_File);
                m_File = nullptr;
            }

            BLoggerString fullPath;
            constructFullPath(fullPath);

            OPEN_FILE(m_File, fullPath);
        }
    };
}
