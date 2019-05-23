#pragma once
#include <stdio.h>
#include <iostream>

#include <string>
#include <sstream>

#include <time.h>
#include <iomanip>

#if defined(__unix__)
	#define UPDATE_TIME localtime_r(&t, &m_BT)
#elif defined(_MSC_VER)
	#define UPDATE_TIME localtime_s(&m_BT, &t)
#endif

namespace level {

	enum level_enum
	{
		TRACE = 0, DEBUG, INFO, WARN, ERROR
	};

}

namespace BLogger {

	class Logger
	{
	private:
		const char* m_Name;
		level::level_enum m_Filter;
		bool m_ShowTimestamps;
		std::tm m_BT;
	public:
		Logger()
		{
			m_Filter = level::TRACE;
			m_ShowTimestamps = false;
		}

		Logger(const char* name)
			: m_Name(name)
		{
			m_Filter = level::TRACE;
			m_ShowTimestamps = false;
		}

		void Log(const char* message, level::level_enum lvl)
		{

			if (m_Filter > lvl)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << enumToString(lvl);
				ss << m_Name << ": "<< message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << enumToString(lvl) << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		template<typename... Args>
		void Log(const char* formattedMsg, level::level_enum lvl, const Args& ... args)
		{
			if (m_Filter > lvl)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << enumToString(lvl);
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << enumToString(lvl) << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		void Trace(const char* message)
		{
			if (m_Filter > level::TRACE)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[TRACE] ";
				ss << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << "[TRACE] " << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		void Debug(const char* message)
		{
			if (m_Filter > level::DEBUG)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[DEBUG] ";
				ss << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << "[DEBUG] " << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		void Info(const char* message)
		{
			if (m_Filter > level::INFO)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[INFO]  ";
				ss << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << "[INFO] " << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		void Warning(const char* message)
		{
			if (m_Filter > level::WARN)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[WARN]  ";
				ss << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << "[WARN] " << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		void Error(const char* message)
		{
			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[ERROR] ";
				ss << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
			else
			{
				std::stringstream ss;
				ss << "[ERROR] " << m_Name << ": " << message << "\n";
				std::cout << ss.str();
			}
		}

		template<typename... Args>
		void Trace(const char* formattedMsg, const Args &... args)
		{
			if (m_Filter > level::TRACE)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[TRACE] ";
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << "[TRACE] " << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		template<typename... Args>
		void Debug(const std::string& formattedMsg, const Args &... args)
		{
			if (m_Filter > level::DEBUG)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[DEBUG] ";
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << "[DEBUG] " << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		template<typename... Args>
		void Info(const std::string& formattedMsg, const Args &... args)
		{
			if (m_Filter > level::INFO)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[INFO]  ";
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << "[INFO] " << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		template<typename... Args>
		void Warning(const std::string& formattedMsg, const Args &... args)
		{
			if (m_Filter > level::WARN)
				return;

			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[WARN]  ";
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << "[WARN] " << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		template<typename... Args>
		void Error(const std::string& formattedMsg, const Args &... args)
		{
			if (m_ShowTimestamps)
			{
				auto t = std::time(nullptr);
				std::stringstream ss;
				UPDATE_TIME;
				ss << std::put_time(&m_BT, "[%OH:%OM:%OS]") << "[ERROR] ";
				ss << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
			else
			{
				std::stringstream ss;
				ss << "[ERROR] " << m_Name << ": " << formattedMsg << "\n";
				printf(ss.str().c_str(), args...);
			}
		}

		void SetFilter(level::level_enum lvl)
		{
			m_Filter = lvl;
		}

		void ShowTimestamps(const bool& setting)
		{
			m_ShowTimestamps = setting;
		}

	private:
		const char* enumToString(level::level_enum lvl)
		{
			switch (lvl)
			{
			case level::TRACE:
				return "[TRACE] ";
			case level::DEBUG:
				return "[DEBUG] ";
			case level::INFO:
				return "[INFO]  ";
			case level::WARN:
				return "[WARN]  ";
			case level::ERROR:
				return "[ERROR] ";
			default:
				return "[UNKNOWN] ";
			}
		}
	};
}