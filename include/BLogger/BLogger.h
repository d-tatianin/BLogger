#pragma once

#include <stdio.h>
#include <iostream>

#include <vector>
#include <string>
#include <sstream>

#include <time.h>
#include <iomanip>

#include <algorithm>

#include "Colors.h"

#ifdef _WIN32
    #define UPDATE_TIME(to, from) localtime_s(&to, &from)
    #define OPEN_FILE(file, path) fopen_s(&file, path.c_str(), "w")
#else
    #define UPDATE_TIME(to, from) localtime_r(&from, &to)
    #define OPEN_FILE(file, path) file = fopen(path.c_str(), "w")
#endif

#define BLOGGER_TRACE_COLOR BLOGGER_WHITE
#define BLOGGER_DEBUG_COLOR BLOGGER_GREEN
#define BLOGGER_INFO_COLOR  BLOGGER_BLUE
#define BLOGGER_WARN_COLOR  BLOGGER_YELLOW
#define BLOGGER_ERROR_COLOR BLOGGER_RED
#define BLOGGER_CRIT_COLOR  BLOGGER_MAGENTA

#define MAX_MESSAGE_SIZE 128
#define BLOGGER_INFINITE 0u

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

class BLogger
{
private:
    std::tm           m_BT;
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
    BLogger() 
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
        m_ShowTag(true),
        m_BT()
    {
    }

    BLogger(const std::string& tag) 
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
        m_ShowTag(true),
        m_BT()
    {
        m_Tag += tag;
        m_Tag += "]";
    }

    BLogger(const std::string& tag, level::level_enum lvl)
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
        m_ShowTag(true),
        m_BT()
    {
        m_Tag += tag;
        m_Tag += "]";
    }

    BLogger(const BLogger& other)            = delete;
    BLogger& operator=(const BLogger& other) = delete;

    BLogger(BLogger&& other)                 = default;
    BLogger& operator=(BLogger&& other)      = default;

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

    void Flush()
    {
        std::cout.flush();

        if (m_File)
            fflush(m_File);
    }

    template <typename T>
    void Log(level::level_enum lvl, const T& message)
    {

        if (m_Filter > lvl)
            return;

        if (!m_LogToConsole && !m_LogToFile)
            return;

        std::stringstream ss;

        if (m_AppendTimestamp)
        {
            auto t = std::time(nullptr);
            UPDATE_TIME(m_BT, t);
            ss << std::put_time(&m_BT, "[%OH:%OM:%OS]");
        }

        ss << LevelToString(lvl);

        if (m_ShowTag)
            ss << m_Tag;

        ss << " " << message << "\n";

        if (m_LogToConsole)
        {
            if (m_ColoredOutput)
            {
                switch (lvl)
                {
                case level::trace: set_output_color(BLOGGER_TRACE_COLOR); break;
                case level::debug: set_output_color(BLOGGER_DEBUG_COLOR); break;
                case level::info:  set_output_color(BLOGGER_INFO_COLOR);  break;
                case level::warn:  set_output_color(BLOGGER_WARN_COLOR);  break;
                case level::error: set_output_color(BLOGGER_ERROR_COLOR); break;
                case level::crit:  set_output_color(BLOGGER_CRIT_COLOR);  break;
                }
            }

            std::cout << ss.str();

            if (m_ColoredOutput)
                set_output_color(BLOGGER_RESET);
        }

        if (m_LogToFile)
        {
            size_t bytes;
            ss.seekg(0, std::ios::beg);
            ss.seekg(0, std::ios::end);
            bytes = static_cast<size_t>(ss.tellg());

            if (m_BytesPerFile && bytes > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + bytes) > m_BytesPerFile)
            {
                if (m_CurrentLogFiles == m_MaxLogFiles)
                {
                    if (!m_RotateLogs)
                        return;
                    else
                    {
                        m_CurrentLogFiles = 1;
                        m_CurrentBytes = 0;
                        NewLogFile();
                    }
                }
                else
                {
                    m_CurrentBytes = 0;
                    ++m_CurrentLogFiles;
                    NewLogFile();
                }

            }

            m_CurrentBytes += bytes;
            fprintf(m_File, ss.str().c_str());
        }
    }

    template<typename T, typename... Args>
    void Log(level::level_enum lvl, const T& formattedMsg, const Args& ... args)
    {
        if (m_Filter > lvl)
            return;

        if (!m_LogToConsole && !m_LogToFile)
            return;

        std::stringstream ss;

        if (m_AppendTimestamp)
        {
            auto t = std::time(nullptr);
            UPDATE_TIME(m_BT, t);
            ss << std::put_time(&m_BT, "[%OH:%OM:%OS]");
        }

        ss << LevelToString(lvl);

        if (m_ShowTag)
            ss << m_Tag;

        ss << " " << formattedMsg << "\n";

        std::vector<char> message;
        message.resize(MAX_MESSAGE_SIZE);
        size_t bytes = static_cast<size_t>(
            snprintf(message.data(), message.size(), ss.str().c_str(), args...)
        );
        ++bytes; // take null terminator into account

        if (m_LogToConsole)
        {
            if (m_ColoredOutput)
            {
                switch (lvl)
                {
                case level::trace: set_output_color(BLOGGER_TRACE_COLOR); break;
                case level::debug: set_output_color(BLOGGER_DEBUG_COLOR); break;
                case level::info:  set_output_color(BLOGGER_INFO_COLOR);  break;
                case level::warn:  set_output_color(BLOGGER_WARN_COLOR);  break;
                case level::error: set_output_color(BLOGGER_ERROR_COLOR); break;
                case level::crit:  set_output_color(BLOGGER_CRIT_COLOR);  break;
                }
            }

            std::cout << message.data();

            if (m_ColoredOutput)
                set_output_color(BLOGGER_RESET);
        }

        if (m_LogToFile)
        {
            if (m_BytesPerFile && bytes > m_BytesPerFile)
                return;

            if (m_BytesPerFile && (m_CurrentBytes + bytes) > m_BytesPerFile)
            {
                if (m_CurrentLogFiles == m_MaxLogFiles)
                {
                    if (!m_RotateLogs)
                        return;
                    else
                    {
                        m_CurrentLogFiles = 1;
                        m_CurrentBytes = 0;
                        NewLogFile();
                    }
                }
                else
                {
                    ++m_CurrentLogFiles;
                    m_CurrentBytes = 0;
                    NewLogFile();
                }
            }

            m_CurrentBytes += bytes;
            fwrite(message.data(), 1, bytes, m_File);
        }
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
    void Trace(const T& formattedMsg, const Args &... args)
    {
        Log(level::trace, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Debug(const T& formattedMsg, const Args &... args)
    {
        Log(level::debug, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Info(const T& formattedMsg, const Args &... args)
    {
        Log(level::info, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Warning(const T& formattedMsg, const Args &... args)
    {
        Log(level::warn, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Error(const T& formattedMsg, const Args &... args)
    {
        Log(level::error, formattedMsg, args...);
    }

    template<typename T, typename... Args>
    void Critical(const T& formattedMsg, const Args &... args)
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

    ~BLogger()
    {
        if (m_File)
            fclose(m_File);
    }

private:
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
};

#undef UPDATE_TIME
#undef OPEN_FILE
#undef BLOGGER_TRACE_COLOR
#undef BLOGGER_DEBUG_COLOR
#undef BLOGGER_INFO_COLOR
#undef BLOGGER_WARN_COLOR
#undef BLOGGER_ERROR_COLOR
#undef MAX_MESSAGE_SIZE
