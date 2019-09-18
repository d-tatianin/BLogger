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
        BLoggerBuffer buf;
        size_t buf_elements;
        std::tm time_point;
        level lvl;
        bool log_to_stdout;
        bool log_to_file;
    public:
        LogMsg(
            BLoggerBuffer&& tBuf,
            size_t size,
            std::tm tp,
            level lvl,
            bool log_stdout,
            bool log_file
        )
            : buf(std::move(tBuf)),
            buf_elements(size),
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
            return buf_elements;
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
        level             m_Filter;
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
            : m_Tag("Unnamed"),
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
            : m_Tag(tag),
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

        BLoggerBase(const std::string& tag, level lvl)
            : m_Tag(tag),
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

        template <typename T = std::string>
        void Log(level lvl, const T& message)
        {
            BLoggerFormatter formatter;

            if (!InitMessage(lvl, formatter))
                return;

            formatter.write_to(
                message, 
                std::strlen(message)
            );

            formatter.newline();

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            post({
                formatter.release_buffer(),
                formatter.size(),
                time_point,
                lvl,
                m_LogToConsole,
                m_LogToFile }
            );
        }

        template<typename... Args>
        void Log(level lvl, const std::string& formattedMsg, Args&& ... args)
        {
            BLoggerFormatter formatter;

            if (!InitMessage(lvl, formatter))
                return;

            formatter.write_to(
                formattedMsg.c_str(), 
                formattedMsg.size()
            );

            BLOGGER_PACK_BEGIN(formatter, args);

            formatter.newline();

            std::tm time_point;
            auto time_now = std::time(nullptr);
            UPDATE_TIME(time_point, time_now);

            post({
                formatter.release_buffer(),
                formatter.size(),
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
        void Trace(const T& formattedMsg, const Args& ... args)
        {
            Log(level::trace, formattedMsg, args...);
        }

        template<typename T, typename... Args>
        void Debug(const T& formattedMsg, const Args& ... args)
        {
            Log(level::debug, formattedMsg, args...);
        }

        template<typename T, typename... Args>
        void Info(const T& formattedMsg, const Args& ... args)
        {
            Log(level::info, formattedMsg, args...);
        }

        template<typename T, typename... Args>
        void Warning(const T& formattedMsg, const Args& ... args)
        {
            Log(level::warn, formattedMsg, args...);
        }

        template<typename T, typename... Args>
        void Error(const T& formattedMsg, const Args& ... args)
        {
            Log(level::error, formattedMsg, args...);
        }

        template<typename T, typename... Args>
        void Critical(const T& formattedMsg, const Args& ... args)
        {
            Log(level::crit, formattedMsg, args...);
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

        bool InitMessage(level lvl, BLoggerFormatter& out_format)
        {
            if (m_Filter > lvl)
                return false;

            if (!m_LogToConsole && !m_LogToFile)
                return false;

            if (m_AppendTimestamp)
                out_format.init_timestamp();

            out_format.append_level(lvl);

            if (m_ShowTag)
                  out_format.append_tag(m_Tag);

            out_format.add_space();

            return true;
        }

        virtual void post(LogMsg&& msg) = 0;
    };
}
