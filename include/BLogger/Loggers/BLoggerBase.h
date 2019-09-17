#pragma once

#pragma once

#include <stdio.h>
#include <iostream>

#include <array>
#include <string>
#include <sstream>


#include <iomanip>

#include <algorithm>

#include "BLogger/OS/Colors.h"
#include "BLogger/OS/Functions.h"


#define BLOGGER_TRACE_COLOR BLOGGER_WHITE
#define BLOGGER_DEBUG_COLOR BLOGGER_GREEN
#define BLOGGER_INFO_COLOR  BLOGGER_BLUE
#define BLOGGER_WARN_COLOR  BLOGGER_YELLOW
#define BLOGGER_ERROR_COLOR BLOGGER_RED
#define BLOGGER_CRIT_COLOR  BLOGGER_MAGENTA

#define MAX_MESSAGE_SIZE 128
#define BLOGGER_INFINITE 0u
#define BLOGGER_TS_PATTERN "%H:%M:%S"

namespace level {

    enum level_enum : int
    {
        trace = 0,
        debug,
        info,
        warn,
        error,
        crit
    };

    inline const char* LevelToString(level_enum lvl)
    {
        switch (lvl)
        {
        case level::trace:  return "[TRACE]";
        case level::debug:  return "[DEBUG]";
        case level::info:   return "[INFO]";
        case level::warn:   return "[WARNING]";
        case level::error:  return "[ERROR]";
        case level::crit:   return "[CRITICAL]";
        default:            return nullptr;
        }
    }
}

class BLoggerBase
{
protected:
    typedef char charT;

    template<size_t size>
    using buffer = std::array<charT, size>;

    template<typename bufferT>
    class BLoggerBuffer
    {
    protected:
        bufferT m_Buffer;
        charT* m_Cursor;
        size_t m_Occupied; 
    public:
        BLoggerBuffer()
            : m_Buffer(),
            m_Cursor(m_Buffer.data()),
            m_Occupied(0)
        {
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

        void write_to_enclosed(const charT* data, size_t size, charT opening = '[', charT closing = ']')
        {
            if ((m_Buffer.size() - m_Occupied) >= size + 2)
            {
                *(m_Cursor++) = opening;
                MEMORY_COPY(m_Cursor, remaining(), data, size);
                m_Cursor += size;
                *(m_Cursor++) = closing;
                m_Occupied += size + 2;
            }
        }

        void add_space()
        {
            if ((m_Buffer.size() - m_Occupied) >= 1)
            {
                *(m_Cursor++) = ' ';
                m_Occupied += 1;
            }
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
    };

    typedef BLoggerBuffer<buffer<MAX_MESSAGE_SIZE>> msgBuffer;

    struct LogMsg
    {
    private:
        msgBuffer buf;
        std::tm time_point;
        level::level_enum lvl;
        bool log_to_stdout;
        bool log_to_file;
    public:
        LogMsg(
            msgBuffer&& tBuf,
            std::tm tp,
            level::level_enum lvl,
            bool log_stdout,
            bool log_file
        )
            : buf(std::move(tBuf)),
            time_point(tp),
            lvl(lvl),
            log_to_stdout(log_stdout),
            log_to_file(log_file)
        {
        }

        charT* data()
        {
            return buf.data();
        }

        std::tm* time_point_ptr()
        {
            return &time_point;
        }

        size_t size()
        {
            return buf.size();
        }

        level::level_enum level()
        {
            return lvl;
        }

        bool file_logger();
    };
protected:
    std::string       m_Tag;
    level::level_enum m_Filter;
    bool              m_AppendTimestamp;
    bool              m_LogToConsole;
    bool              m_LogToFile;
    bool              m_RotateLogs;
    bool              m_ColoredOutput;
    bool              m_ShowTag;
    FILE*             m_File;
    std::string       m_DirectoryPath;
    size_t            m_BytesPerFile;
    size_t            m_CurrentBytes;
    size_t            m_MaxLogFiles;
    size_t            m_CurrentLogFiles;
public:
    BLoggerBase()
        : m_Tag("[Unnamed]"),
        m_Filter(level::trace),
        m_AppendTimestamp(false),
        m_LogToConsole(false),
        m_LogToFile(false),
        m_ColoredOutput(false),
        m_File(nullptr),
        m_BytesPerFile(0),
        m_CurrentBytes(0),
        m_MaxLogFiles(0),
        m_CurrentLogFiles(0),
        m_RotateLogs(false),
        m_ShowTag(true)
    {
    }

    BLoggerBase(const std::string& tag)
        : m_Tag("["),
        m_Filter(level::trace),
        m_AppendTimestamp(false),
        m_LogToConsole(false),
        m_LogToFile(false),
        m_ColoredOutput(false),
        m_File(nullptr),
        m_BytesPerFile(0),
        m_CurrentBytes(0),
        m_MaxLogFiles(0),
        m_CurrentLogFiles(0),
        m_RotateLogs(false),
        m_ShowTag(true)
    {
        m_Tag += tag;
        m_Tag += "]";
    }

    BLoggerBase(const std::string& tag, level::level_enum lvl)
        : m_Tag("["),
        m_Filter(lvl),
        m_AppendTimestamp(false),
        m_LogToConsole(false),
        m_LogToFile(false),
        m_ColoredOutput(false),
        m_File(nullptr),
        m_BytesPerFile(0),
        m_CurrentBytes(0),
        m_MaxLogFiles(0),
        m_CurrentLogFiles(0),
        m_RotateLogs(false),
        m_ShowTag(true)
    {
        m_Tag += tag;
        m_Tag += "]";
    }

    BLoggerBase(const BLoggerBase& other) = delete;
    BLoggerBase& operator=(const BLoggerBase& other) = delete;

    BLoggerBase(BLoggerBase&& other) = default;
    BLoggerBase& operator=(BLoggerBase&& other) = default;

    bool InitFileLogger(const char* directoryPath, size_t bytesPerFile, size_t maxLogFiles, bool rotateLogs = true)
    {
        m_BytesPerFile = bytesPerFile;
        m_MaxLogFiles = maxLogFiles;
        m_RotateLogs = rotateLogs;
        m_CurrentBytes = 0;
        m_CurrentLogFiles = 1;

        m_DirectoryPath = directoryPath;
        m_DirectoryPath += '/';

        std::string fullPath;
        ConstructFullPath(fullPath);

        OPEN_FILE(m_File, fullPath);

        if (m_File)
            return true;

        Error("Could not initialize the file logger! Make sure the path is valid.");
        return false;
    }

    bool EnableFileLogger()
    {
        if (!m_File)
        {
            Error("Could not enable the file logger. Did you call InitFileLogger?");
            return false;
        }

        m_LogToFile = true;
        return true;
    }

    void DisableFileLogger()
    {
        m_LogToFile = false;
    }

    void TerminateFileLogger()
    {
        m_LogToFile = false;

        if (m_File)
        {
            fclose(m_File);
            m_File = nullptr;
        }
    }

    void EnableConsoleLogger()
    {
        m_LogToConsole = true;
    }

    void DisableConsoleLogger()
    {
        m_LogToConsole = false;
    }

    void EnableColoredOutput()
    {
        m_ColoredOutput = true;
    }

    void DisableColoredOutput()
    {
        m_ColoredOutput = false;
    }

    void EnableTimestamps()
    {
        m_AppendTimestamp = true;
    }

    void DisableTimestamps()
    {
        m_AppendTimestamp = false;
    }

    void EnableTag()
    {
        m_ShowTag = true;
    }

    void DisableTag()
    {
        m_ShowTag = false;
    }

    virtual void Flush() = 0;

    template <typename T>
    void Log(level::level_enum lvl, const T& message)
    {
        msgBuffer buffer;

        if (!InitMessage(lvl, buffer))
            return;

       buffer.write_to(message, strlen(message));

       buffer.newline();

       std::tm time_point;
       auto time_now = std::time(nullptr);
       UPDATE_TIME(time_point, time_now);

       post({ 
           std::move(buffer), 
           time_point, 
           lvl, 
           m_LogToConsole, 
           m_LogToFile }
       );
    }

    template<typename T, typename... Args>
    void Log(level::level_enum lvl, const T& formattedMsg, const Args& ... args)
    {
        msgBuffer buffer;

        if (!InitMessage(lvl, buffer))
            return;

        buffer.advance_cursor_by(
            snprintf(
                buffer.cursor(),
                buffer.remaining(),
                formattedMsg,
                args...
            ));
        buffer.newline();

        std::tm time_point;
        auto time_now = std::time(nullptr);
        UPDATE_TIME(time_point, time_now);

        post({
            std::move(buffer),
            time_point,
            lvl,
            m_LogToConsole,
            m_LogToFile }
        );
    }

    template <typename T>
    void Trace(const T& message)
    {
        Log(level::trace, message);
    }

    template <typename T>
    void Debug(const T& message)
    {
        Log(level::debug, message);
    }

    template <typename T>
    void Info(const T& message)
    {
        Log(level::info, message);
    }

    template <typename T>
    void Warning(const T& message)
    {
        Log(level::warn, message);
    }

    template <typename T>
    void Error(const T& message)
    {
        Log(level::error, message);
    }

    template <typename T>
    void Critical(const T& message)
    {
        Log(level::crit, message);
    }

    template<typename T, typename... Args>
    void Trace(const T& formattedMsg, const Args&... args)
    {
        Log(level::trace, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Debug(const T& formattedMsg, const Args&... args)
    {
        Log(level::debug, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Info(const T& formattedMsg, const Args&... args)
    {
        Log(level::info, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Warning(const T& formattedMsg, const Args&... args)
    {
        Log(level::warn, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Error(const T& formattedMsg, const Args&... args)
    {
        Log(level::error, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Critical(const T& formattedMsg, const Args&... args)
    {
        Log(level::crit, formattedMsg, args...);
    }

    void SetFilter(level::level_enum lvl)
    {
        m_Filter = lvl;
    }

    void SetTag(const std::string& tag)
    {
        m_Tag = "[";
        m_Tag += tag;
        m_Tag += "]";
    }

    virtual ~BLoggerBase()
    {
        if (m_File)
            fclose(m_File);
    }

protected:
    void ConstructFullPath(std::string& outPath)
    {
        outPath += m_DirectoryPath;
        outPath += std::string(m_Tag.begin() + 1, m_Tag.end() - 1);
        outPath += '-';
        outPath += std::to_string(m_CurrentLogFiles);
        outPath += ".log";
    }

    void NewLogFile()
    {
        fclose(m_File);
        m_File = nullptr;

        std::string fullPath;
        ConstructFullPath(fullPath);

        OPEN_FILE(m_File, fullPath);
    }

    bool InitMessage(level::level_enum lvl, msgBuffer& out_buffer)
    {
        if (m_Filter > lvl)
            return false;

        if (!m_LogToConsole && !m_LogToFile)
            return false;

        if (m_AppendTimestamp)
            out_buffer.write_to_enclosed(
                BLOGGER_TS_PATTERN,
                strlen(BLOGGER_TS_PATTERN)
            );

        out_buffer.write_to(
            LevelToString(lvl),
            strlen(LevelToString(lvl))
        );

        if (m_ShowTag)
            out_buffer.write_to(
                m_Tag.c_str(), 
                m_Tag.size()
            );

        out_buffer.add_space();

        return true;
    }

    virtual void post(LogMsg&& msg) = 0;
};

#undef MAX_MESSAGE_SIZE
