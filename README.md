# BLogger
An easy to use logger, which supports text formatting, time stamps as well as operator overloading.

## Using the logger:
### - Create an instance of BLogger by calling one of the constructors. 
- `BLogger::Logger()` -> Sets the logger name to `"Unknown"` and the logging filter to `TRACE`.
- `BLogger::Logger(const std::string& name)` -> Sets the logger name to `name` and the logging filter to `TRACE`.
- `BLogger::Logger(const std::string& name, level lvl)` -> Sets the logger name to `name` and the logging filter to `lvl`.
---
### - Logging your messages:
- `Log(const T& message, level lvl)` -> Logs the message with the given level. Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::stringstream`.
- `Log(const T& formattedMsg, level lvl, const Args& ... args)` -> Logs the message with the given level. Expects a `printf`-style formatted string as well as any arguments to go with it.
### - The following redundant member functions are also available with the same overloads as `Log()`, however, don't require a level argument:
- `Trace(...)` -> Logs the given message with logging level `TRACE`.
- `Debug(...)`-> Logs the given message with logging level `DEBUG`.
- `Info(...)` -> Logs the given message with logging level `INFO`.
- `Warning(...)` -> Logs the given message with logging level `WARN`.
- `Error(...)` -> Logs the given message with logging level `Error`.
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
- `Error` -> changed from `ERROR` in order to resolve a conflict with the Windows header.
