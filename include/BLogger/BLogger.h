#pragma once

#include <stdio.h>
#include <iostream>

#include <vector>
#include <string>
#include <sstream>

#include <time.h>
#include <iomanip>

#include <algorithm>

#if defined(__unix__)
	#define UPDATE_TIME localtime_r(&t, &m_BT)
#elif defined(_MSC_VER)
	#define UPDATE_TIME localtime_s(&m_BT, &t)
#endif

#define MAX_MESSAGE_SIZE 128

namespace level {

	enum level_enum : int
	{
		TRACE = 0, 
		DEBUG, 
		INFO, 
		WARN, 
		CRIT
	};

	inline const char* LevelToString(level_enum lvl)
	{
		switch (lvl)
		{
		case level::TRACE:
			return "[TRACE] ";
		case level::DEBUG:
			return "[DEBUG] ";
		case level::INFO:
			return "[INFO] ";
		case level::WARN:
			return "[WARNING] ";
		case level::CRIT:
			return "[CRITICAL] ";
		default:
			return nullptr;
		}
	}
}

class BLogger
{
private:
	std::tm           m_BT;
	std::string       m_Name;
	level::level_enum m_Filter;
	bool              m_AppendTimestamp;
	bool              m_LogToConsole;
	bool              m_LogToFile;
	bool              m_RotateLogs;
	FILE*             m_File;
	std::string       m_DirectoryPath;
	size_t            m_BytesPerFile;
	size_t            m_CurrentBytes;
	size_t            m_MaxLogFiles;
	size_t            m_CurrentLogFiles;
public:
	BLogger() 
		: m_Name("Unnamed"), 
		  m_Filter(level::TRACE), 
		  m_AppendTimestamp(false),
		  m_LogToConsole(false),
		  m_LogToFile(false),
		  m_File(nullptr),
		  m_BytesPerFile(0),
		  m_CurrentBytes(0),
		  m_MaxLogFiles(0),
		  m_CurrentLogFiles(0)
	{
	}

	BLogger(const std::string& name) 
		: m_Name(name),
		m_Filter(level::TRACE),
		m_AppendTimestamp(false),
		m_LogToConsole(false),
		m_LogToFile(false),
		m_File(nullptr),
		m_BytesPerFile(0),
		m_CurrentBytes(0),
		m_MaxLogFiles(0),
		m_CurrentLogFiles(0)
	{
	}

	BLogger(const std::string& name, level::level_enum lvl)
		: m_Name(name),
		m_Filter(lvl),
		m_AppendTimestamp(false),
		m_LogToConsole(false),
		m_LogToFile(false),
		m_File(nullptr),
		m_BytesPerFile(0),
		m_CurrentBytes(0),
		m_MaxLogFiles(0),
		m_CurrentLogFiles(0)
	{
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
		
		fopen_s(&m_File, fullPath.c_str(), "w");

		if (m_File)
			return true;
		
		Critical("Could not initialize the file logger! Make sure the path is valid.");
		return false;	
	}

	bool EnableFileLogger()
	{
		if (!m_File)
		{
			Critical("Could not enable the file logger. Did you call InitFileLogger?");
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
	
	template <typename T>
	void Log(level::level_enum lvl, const T& message)
	{

		if (m_Filter > lvl)
			return;

		if (!m_LogToConsole && !m_LogToFile)
			return;

		auto t = std::time(nullptr);
		UPDATE_TIME;
		std::stringstream ss;
		
		if (m_AppendTimestamp)
			ss << std::put_time(&m_BT, "[%OH:%OM:%OS]");

		ss << LevelToString(lvl);
		ss << m_Name << ": " << message << "\n";

		if (m_LogToConsole)
		{
			std::cout << ss.str();
		}

		if (m_LogToFile)
		{
			size_t bytes;
			ss.seekg(0, std::ios::beg);
			ss.seekg(0, std::ios::end);
			bytes = ss.tellg();

			if (bytes > m_BytesPerFile)
				return;

			if ((m_CurrentBytes + bytes) > m_BytesPerFile)
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

		auto t = std::time(nullptr);
		UPDATE_TIME;
		std::stringstream ss;

		if (m_AppendTimestamp)
			ss << std::put_time(&m_BT, "[%OH:%OM:%OS]");

		ss << LevelToString(lvl);
		ss << m_Name << ": " << formattedMsg << "\n";

		std::vector<char> message;
		message.resize(MAX_MESSAGE_SIZE);
		snprintf(message.data(), message.size(), ss.str().c_str(), args...);
		
		auto itr = std::remove_if(message.begin(), message.end(), [](char c) { return c == '\0'; });
		*itr = '\0';
		message.erase(++itr, message.end());

		if (m_LogToConsole)
		{
			std::cout << message.data();
		}

		if (m_LogToFile)
		{
			size_t bytes = message.size();

			if (bytes > m_BytesPerFile)
				return;

			if ((m_CurrentBytes + bytes) > m_BytesPerFile)
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
			fprintf(m_File, message.data());
		}
	}

	template <typename T>
	void Trace(const T& message)
	{
		Log(level::TRACE, message);
	}

	template <typename T>
	void Debug(const T& message)
	{
		Log(level::DEBUG, message);
	}

	template <typename T>
	void Info(const T& message)
	{
		Log(level::INFO, message);
	}

	template <typename T>
	void Warning(const T& message)
	{
		Log(level::WARN, message);
	}

	template <typename T>
	void Critical(const T& message)
	{
		Log(level::CRIT, message);
	}

	template<typename T, typename... Args>
	void Trace(const T& formattedMsg, const Args &... args)
	{
		Log(level::TRACE, formattedMsg, args...);
	}

	template<typename T, typename... Args>
	void Debug(const T& formattedMsg, const Args &... args)
	{
		Log(level::DEBUG, formattedMsg, args...);
	}

	template<typename T, typename... Args>
	void Info(const T& formattedMsg, const Args &... args)
	{
		Log(level::INFO, formattedMsg, args...);
	}

	template<typename T, typename... Args>
	void Warning(const T& formattedMsg, const Args &... args)
	{
		Log(level::WARN, formattedMsg, args...);
	}

	template<typename T, typename... Args>
	void Critical(const T& formattedMsg, const Args &... args)
	{
		Log(level::CRIT, formattedMsg, args...);
	}

	void SetFilter(level::level_enum lvl)
	{
		m_Filter = lvl;
	}

	void ShowTimestamps(bool setting)
	{
		m_AppendTimestamp = setting;
	}

	void SetName(const std::string& name)
	{
		m_Name = name;
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
		outPath += m_Name;
		outPath += '-';
		outPath += std::to_string(m_CurrentLogFiles);
		outPath += ".txt";
	}

	void NewLogFile()
	{
		fclose(m_File);
		m_File = nullptr;

		std::string fullPath;
		ConstructFullPath(fullPath);

		fopen_s(&m_File, fullPath.c_str(), "w");
	}
};
