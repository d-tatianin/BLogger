# BLogger
An easy to use logger which supports console & file logging, log rotation and more!

## Using the logger:
### - Create an instance of BLogger by calling one of the constructors. 
- `BLogger()` -> Sets the logger name to `"Unnamed"` and the logging filter to `TRACE`.
- `BLogger(const std::string& name)` -> Sets the logger name to `name` and the logging filter to `TRACE`.
- `BLogger(const std::string& name, level lvl)` -> Sets the logger name to `name` and the logging filter to `lvl`.
---
### - Setting up the loggers:
By default, the console and file loggers are disabled.  
In order to activate the console logger use `EnableConsoleLogger()`.  
In order to deactivate the console logger again use `DisableConsoleLogger()`.  
That's it for console logger!

Activating the file logger is a bit more tricky, however, not that difficult as well.  
In order to activate file logger you have to first initialize it.  
In order to initialize the file logger call:  
`InitFileLogger(const char* directoryPath, size_t bytesPerFile, size_t maxLogFiles, bool rotateLogs = true)`  

1. A valid directory path where you want the logs to be store, e.g `"C:\mylogs"`.  
2. Byte limit per logging file.  
3. Maximum number of logging files, so your total byte limit is `bytesPerFile` * `maxLogFiles`.  
4. An optional parameter, if it's set to `true` once the logger reaches the `maxLogFiles` limit it will go back to the first log file and overwrite it with new logs.  

In case `InitFileLogger(...)` fails to initialize the file logger, it will return `false` and log an error message. If it succeeds, `true` is returned.  

After you successfully initialized the file logger, you need to call `EnableFileLogger()` to enable it. As a fail-safe, it also returns `true` if the operation succeeded, or `false`, as well as an error message otherwise.

File logger logfile naming pattern: `loggerName + loggingFileNumber + .log`, e.g `mylogger-1.log`.  

In order to temporarily disable the file logger, call `DisableFileLogger()`.  
If you do not intend to use the file logger anymore, use `TerminateFileLogger()`. Keep in mind that you would have to call `InitFileLogger(...)` again if you ever want to reactivate it.  

---
### - Logging your messages:
- `Log(level lvl, const T& message)` -> Logs the message with the given level.  
- `Log(level lvl, const T& formattedMsg, const Args& ... args)` -> Logs the message with the given level. Expects a `printf`-style formatted string as well as any arguments to go with it. Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::stringstream`.  
### - The following redundant member functions are also available with the same overloads as `Log()`, however, don't require a level argument:
- `Trace(...)` -> Logs the given message with logging level `TRACE`.
- `Debug(...)`-> Logs the given message with logging level `DEBUG`.
- `Info(...)` -> Logs the given message with logging level `INFO`.
- `Warning(...)` -> Logs the given message with logging level `WARN`.
- `Critical(...)` -> Logs the given message with logging level `CRIT`.
---
### - Misc member functions:
- `SetFilter(level lvl)` - > Sets the logging filter to the level specified.
- `SetName(const std::string& name)` -> Sets the logger name to the name specified.
- `ShowTimespamps(bool setting)` -> Changes whether the logger displays timestamps before the message. Set to `false` by default.
---
### There is a total of 5 available logging levels that reside inside the unscoped level_enum inside the level namespace:
- `TRACE`
- `DEBUG`
- `INFO`
- `WARN`
- `CRIT`
