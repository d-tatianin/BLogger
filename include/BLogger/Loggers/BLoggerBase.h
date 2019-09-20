#pragma once

#include <stdio.h>
#include <ctime>
#include <iostream>

#include <array>
#include <string>
#include <sstream>

#include "BLogger/LogLevels.h"
#include "BLogger/Formatter/Formatter.h"
#include "BLogger/OS/Colors.h"
#include "BLogger/OS/Functions.h"

#define BLOGGER_INFINITE 0u

namespace BLogger {

    struct LogMsg
    {
    private:
        BLoggerFormatter fmt;
        BLoggerPattern ptrn;
        std::tm time_point;
        level lvl;
        bool log_to_stdout;
        bool log_to_file;
    public:
        LogMsg(
            BLoggerFormatter&& fmt,
            BLoggerPattern ptrn,
            std::tm tp,
            level lvl,
            bool log_stdout,
            bool log_file
        )
            : fmt(std::move(fmt)),
            ptrn(ptrn),
            time_point(tp),
            lvl(lvl),
            log_to_stdout(log_stdout),
            log_to_file(log_file)
        {
        }

        void finalize_format()
        {
            fmt.merge_pattern(ptrn, time_point_ptr(), lvl);
        }

        charT* data()
        {
            return fmt.data();
        }

        std::tm* time_point_ptr()
        {
            return &time_point;
        }

        size_t size()
        {
            return fmt.size();
        }

        level log_level()
        {
            return lvl;
        }
    };

    class BLoggerBase
    {
    protected:
        std::string       m_Tag;
        std::string       m_DirectoryPath;
        BLoggerPattern    m_Pattern;
        level             m_Filter;
        bool              m_LogToConsole;
        bool              m_LogToFile;
        bool              m_RotateLogs;
        bool              m_ColoredOutput;
        FILE*             m_File;
        size_t            m_BytesPerFile;
        size_t            m_CurrentBytes;
        size_t            m_MaxLogFiles;
        size_t            m_CurrentLogFiles;
    public:
        BLoggerBase()
            : m_Tag("Unnamed"),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false),
            m_File(nullptr),
            m_BytesPerFile(0),
            m_CurrentBytes(0),
            m_MaxLogFiles(0),
            m_CurrentLogFiles(0),
            m_RotateLogs(false)
        {
        }

        BLoggerBase(const std::string& tag)
            : m_Tag(tag),
            m_Filter(level::trace),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false),
            m_File(nullptr),
            m_BytesPerFile(0),
            m_CurrentBytes(0),
            m_MaxLogFiles(0),
            m_CurrentLogFiles(0),
            m_RotateLogs(false)
        {
        }

        BLoggerBase(const std::string& tag, level lvl)
            : m_Tag(tag),
            m_Filter(lvl),
            m_LogToConsole(false),
            m_LogToFile(false),
            m_ColoredOutput(false),
            m_File(nullptr),
            m_BytesPerFile(0),
            m_CurrentBytes(0),
            m_MaxLogFiles(0),
            m_CurrentLogFiles(0),
            m_RotateLogs(false)
        {
        }

        BLoggerBase(const BLoggerBase& other) = delete;
        BLoggerBase& operator=(const BLoggerBase& other) = delete;

        BLoggerBase(BLoggerBase&& other) = default;
        BLoggerBase& operator=(BLoggerBase&& other) = default;

        void SetPattern(const std::string& pattern)
        {
            m_Pattern.init();
            m_Pattern.set_pattern(pattern, m_Tag);
        }

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

        virtual void Flush() = 0;

        template <typename T = std::string>
        void Log(level lvl, const T& message)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                message, 
                std::strlen(message)
            );

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                std::move(formatter),
                m_Pattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile 
            });
        }

        template<typename... Args>
        void Log(level lvl, const std::string& formattedMsg, Args&& ... args)
        {
            BLoggerFormatter formatter;

            if (!ShouldLog(lvl))
                return;

            formatter.process_message(
                formattedMsg.c_str(), 
                formattedMsg.size()
            );

            BLOGGER_PROCESS_PACK(formatter, args);

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            Post({
                std::move(formatter),
                m_Pattern,
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile
            });
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
        void Trace(const T& formattedMsg, Args&& ... args)
        {
            Log(level::trace, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Debug(const T& formattedMsg, Args&& ... args)
        {
            Log(level::debug, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Info(const T& formattedMsg, Args&& ... args)
        {
            Log(level::info, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Warning(const T& formattedMsg, Args&& ... args)
        {
            Log(level::warn, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Error(const T& formattedMsg, Args&& ... args)
        {
            Log(level::error, formattedMsg, std::forward<Args>(args)...);
        }

        template<typename T, typename... Args>
        void Critical(const T& formattedMsg, Args&& ... args)
        {
            Log(level::crit, formattedMsg, std::forward<Args>(args)...);
        }

        void SetFilter(level lvl)
        {
            m_Filter = lvl;
        }

        void SetTag(const std::string& tag)
        {
            m_Tag = tag;
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
            outPath += std::string(m_Tag.begin(), m_Tag.end());
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

        bool ShouldLog(level lvl)
        {
            if (m_Filter > lvl)
                return false;

            if (!m_LogToConsole && !m_LogToFile)
                return false;

            return true;
        }

        virtual void Post(LogMsg&& msg) = 0;
    };
}
