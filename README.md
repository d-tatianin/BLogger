# BLogger | [![Build status](https://ci.appveyor.com/api/projects/status/nbwtd4mu4cjmnjcm?svg=true)](https://ci.appveyor.com/project/8infy/blogger) | [![Codacy Badge](https://api.codacy.com/project/badge/Grade/19f939802f724ad4a53854068325f0a3)](https://www.codacy.com/app/8infy/BLogger?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=8infy/BLogger&amp;utm_campaign=Badge_Grade) |

An easy to use modern C++14/17 cross-platform logger which supports custom formatting/pattern, colored output console, file logging, log rotation & more!

## WIP notice
Please note that the logger is currently under active development and is missing some features.  
Some of the features to be released soon:
-   An asynchronous version of BLogger.
-   Setting a pattern like `{ts}{lvl}{tag}{msg}` instead of enabling/disabling parts of the hardcoded pattern manually.
## Using the logger
### - Create an instance of BLogger by calling one of the constructors 
-   `BlockingLogger()` -> Sets the logger name to `"Unnamed"` and the logging filter to `trace`.
-   `BlockingLogger(const std::string& tag)` -> Sets the logger name to `name` and the logging filter to `trace`.
-   `BlockingLogger(const std::string& tag, level lvl)` -> Sets the logger name to `name` and the logging filter to `lvl`.
---
### - Setting up the console logger
By default, the console logger is disabled.  
Activate & decativate with `EnableConsoleLogger()` & `DisableConsoleLogger()` respectively.
### - Printing with color
Console logger can print its messages with a specific color for each logging level, which can be set in LogLevels.h.
Currently supported colors:
-   `BLOGGER_BLACK`
-   `BLOGGER_RED`
-   `BLOGGER_ORANGE`
-   `BLOGGER_BLUE`
-   `BLOGGER_GREEN`
-   `BLOGGER_CYAN`
-   `BLOGGER_MAGENTA`
-   `BLOGGER_YELLOW`
-   `BLOGGER_WHITE`
-   `BLOGGER_RESET` -> used to reset the sink to its default color.
-   `BLOGGER_DEFAULT` -> same as `BLOGGER_RESET`.

Current stdout color is set with `set_output_color(blogger_color color)` defined inside Colors.h (included in BLogger.h), it's automatically managed by BLogger, however, feel free to use it for your own purposes.  
If you want the console logger messages to be colored, call `EnableColoredOutput()`.  
Similarly, if you wish to deactivate colored output, use `DisableColoredOutput()`.  
That's it for console logger!

### - Setting up the file logger
Activating the file logger is a bit more tricky, however, not that difficult as well.  
In order to activate file logger you have to first initialize it.  
In order to initialize the file logger call:  
`InitFileLogger(const char* directoryPath, size_t bytesPerFile, size_t maxLogFiles, bool rotateLogs = true)`  
1.  A valid directory path where you want the logs to be store, e.g `"C:\mylogs"`.  
2.  Byte limit per logging file. You can also use `BLOGGER_INFINITE` for unlimited size.
3.  Maximum number of logging files, so your total byte limit is `bytesPerFile` * `maxLogFiles`.  
4.  An optional parameter, if it's set to `true` once the logger reaches the `maxLogFiles` limit it will go back to the first log file and overwrite it with new logs.  

In case `InitFileLogger(...)` fails to initialize the file logger, it will return `false` and log an error message. If it succeeds, `true` is returned.  

After you successfully initialized the file logger, you need to call `EnableFileLogger()` to enable it. As a fail-safe, it also returns `true` if the operation succeeded, or `false`, as well as an error message otherwise.   

File logger logfile naming pattern: `loggerName + loggingFileNumber + .log`, e.g `mylogger-1.log`.  

In order to temporarily disable the file logger, call `DisableFileLogger()`.  

If you do not intend to use the file logger anymore, use `TerminateFileLogger()`.   
Keep in mind that you would have to call `InitFileLogger(...)` again if you ever want to reactivate it.  

Please note: file logger does not flush after every log message due to performance costs. It only flushes once the file is closed, that is if it's either full (`bytesPerFile` limit reached) OR you called `TerminateFileLogger()` OR logger got destroyed (via delete or automatically). 

---
### - Logging your messages
-   `Log(level lvl, const T& message)` -> Logs the message with the given level.  
-   `Log(level lvl, const T& formattedMsg, const Args& ... args)` -> Logs the formatted message with the given level.

### - BLogger log message formatting
BLogger accepts the following formats:
-   `{}` a normal argument. Usage example: `logger.Critical("Something went wrong {}", error.message());`.
-   `{n}` a positional argument. Usage example: `logger.Info("{1} / {0} = 2", 4, 8)` -> prints `8 / 4 = 2`.
-   You can also mix the two types like so `logger.Info("{1} / {0} = {}", 4, 8, 2)` -> prints `8 / 4 = 2`.  

Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::ostream`.

### - The following redundant member functions are also available with the same overloads as `Log()`, however, don't require a level argument
-   `Trace(...)` -> Logs the given message with logging level `trace`.
-   `Debug(...)`-> Logs the given message with logging level `debug`.
-   `Info(...)` -> Logs the given message with logging level `info`.
-   `Warning(...)` -> Logs the given message with logging level `warn`.
-   `Error(...)` -> Logs the given message with logging level `error`.
-   `Critical(...)` -> Logs the given message with logging level `crit`.
---
### - Misc member functions
-   `SetFilter(level lvl)` - > Sets the logging filter to the level specified.
-   `SetTag(const std::string& tag)` -> Sets the logger name to the name specified.
-   `EnableTimestamps()` -> Enables timestamps before the message.
-   `DisableTimestamps()` -> Disables timestamps before the message.
-   `EnableTag()` -> Enables tags before the message. On by default.
-   `DisableTag()` -> Disables tags before the message.
-   `Flush()` -> Flushes the logger.
---
### There is a total of 6 available logging levels that reside inside the unscoped level_enum inside the level namespace
-   `trace`
-   `debug`
-   `info`
-   `warn`
-   `error`
-   `crit`
