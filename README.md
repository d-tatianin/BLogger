# BLogger | [![Build status](https://ci.appveyor.com/api/projects/status/nbwtd4mu4cjmnjcm?svg=true)](https://ci.appveyor.com/project/8infy/blogger) | [![Codacy Badge](https://api.codacy.com/project/badge/Grade/19f939802f724ad4a53854068325f0a3)](https://www.codacy.com/app/8infy/BLogger?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=8infy/BLogger&amp;utm_campaign=Badge_Grade) |

An easy to use modern C++14/17 async cross-platform logger which supports custom formatting/patterns, colored output, Unicode, file logging, log rotation & more!

## Performance
BlockingLogger
1. Debug: 90μs/message (~11,111/sec).
2. Release: 77μs/message (~12,987/sec).  

AsyncLogger
1. Debug: 8μs/message (~125,000/sec).
2. Release: 0.35μs/message (~2,857,142/sec).

The tests were done with a single-argument message, and a full pattern, as well as a colored StdoutSink.

RAM usage peakes at about 6MB with a full queue (10,000 log messages by default). This can obviously be different for you depending on how long your log message is.
## Building the Example project
1. Clone the repository `git clone https://github.com/8infy/BLogger`
2. Build the project `cd BLogger && mkdir build && cd build && cmake .. && cmake --build .` 

In order to use BLogger in your own project simply add BLogger's include folder into your project's include directories.
## Using the logger  
The preferred way of creating a logger is using the logger factory in combination with `bl::Props` since the factory initializes static BLogger variables in the correct order, which is safer. 
The factory always returns a shared pointer to the logger.  

Currently available factory functions:
-   `bl::Logger::CreateFromProps(BLoggerProps& props)` -> create a custom logger according to the properties passed to the function.
-   `bl::Logger::CreateAsyncConsole(std::string tag, level lvl, bool default_pattern, bool colored)` -> a shortcut for creating an async console logger.
-   `bl::Logger::CreateBlockingConsole(std::string tag, level lvl, bool default_pattern, bool colored)` -> a shortcut for creating a blocking console logger.

So in your code you would write: `auto logger = bl:Logger::CreateAsyncConsole("MyLogger", level::trace);`  

You can also create the logger by calling the constructor directly, and then adding a sink you want by calling `AddSink(SinkPtr sink)` with your sink.

`BLoggerProps` properties:
-   `bool async` -> Creates an async logger if true, blocking otherwise.
-   `bool console_logger` -> Adds an stdout sink if set to true.
-   `bool colored` -> Makes the stdout sink colored if set to true.
-   `std::string tag` -> Current logger name.
-   `std::string pattern` -> Create the logger with this pattern, or uses the default one if empty.
-   `level filter` -> Logging filter.
-   `bool file_logger` -> Adds a file sink if set to true.
-   `std::string path` -> Path to a directory where you what the logs to be stored.
-   `size_t bytes_per_file` -> Maximum bytes per log file. Use `BLOGGER_INFINITE` for unlimited size.
-   `size_t log_files` -> Maximum log files.
-   `bool rotate_logs` -> Overwrites the oldest log file if the limit is hit.

---
### - Setting the pattern  
Arguments you can use for creating a custom pattern:
-   `{ts}` -> timestamp.
-   `{lvl}` -> logging level of the current message.
-   `{tag}` -> logger tag(name).
-   `{msg}` -> the message itself.  

After you've decided on your pattern you can set it by calling `SetPattern(const std::string& pattern)`.

Here's an example of an interesting pattern `"[{ts}][{tag}]\n[{lvl}] -> {msg}\n"`, which looks like this:
![alt-text](https://i.ibb.co/w0yfBcL/BLogger.png)

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

Current stdout color is set with `set_output_color(color color)` defined inside Colors.h, it's automatically managed by BLogger, however, feel free to use it for your own purposes.  

---
### - Logging your messages
-   `Log(level lvl, std::string message)` -> Logs the message with the given level.  
-   `Log(level lvl, std::string formattedMsg, const Args& ... args)` -> Logs the formatted message with the given level.

### - BLogger log message formatting
BLogger accepts the following formats:
-   `{}` a normal argument. Usage example: `logger->Critical("Something went wrong {}", error.message());`.
-   `{n}` a positional argument. Usage example: `logger->Info("{1}, {0}!", "World", "Hello")` -> prints `Hello, World`.
-   You can also mix the two types like so `logger->Info("{} -> {1}, {0} ", "World", "Hello", "TEST")` -> prints `TEST -> Hello, World`.  

Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::ostream`.

### - The following redundant member functions are also available with the same overloads as `Log()`, however, don't require a level argument
-   `Trace(...)` -> Logs the given message with logging level `trace`.
-   `Debug(...)`-> Logs the given message with logging level `debug`.
-   `Info(...)` -> Logs the given message with logging level `info`.
-   `Warning(...)` -> Logs the given message with logging level `warn`.
-   `Error(...)` -> Logs the given message with logging level `error`.
-   `Critical(...)` -> Logs the given message with logging level `crit`.
--- 
### - Unicode logging  
-   In order to enable Unicode mode type `#define BLOGGER_UNICODE_MODE` before including BLogger.h in any translation unit (aka .cpp).  
-   When Unicode mode is enabled, all functions expect wide strings and `const wchar_t*` for literals, wide string literal is defined by typing `L` before the opening quotes. Same way `std::ostream` is replaced with `std::wostream` for passing a user defined data type. As a way to make your life easier BLogger offers a bunch of useful defines like `BLOGGER_WIDEN_IF_NEEDED(string)`, `BLOGGER_OSTREAM`, `BLOGGER_COUT` that are safe to use in both modes. 
---
### - Misc member functions
-   `SetFilter(level lvl)` - > Sets the logging filter to the level specified.
-   `SetTag(const std::string& tag)` -> Sets the logger name to the name specified.
-   `Flush()` -> Flushes the logger.
-   `AddSink(BLoggerSinkPtr sink)` -> Adds a sink to the logger. Not recommended to use this function directly, use a factory instead.
-   `StdoutSink::GetGlobalWriteLock()` -> returns the global mutex BLogger uses to write to a global sink. Use this mutex if you want to combine using BLogger with raw calls to `std::cout`. If you lock the mutex before writing to a global sink your message is guaranteed to be properly printed and be the default color.
-   `Formatter::CutIfExceeds(size_t size, std::string postfix)` -> Sets the maximum size of a log message. If the message exceeeds the set size it will be cut and the postfix will be inserted after. The postfix is set to `"..."` by default. Size can also be set to `BLOGGER_INFINITE`, which is the default setting.
-   `Formatter::SetTimestampFormat(std::string new_format)` -> Sets the timestamp format. Should be formatted according to the `strftime` specifications.
---
### There is a total of 6 available logging levels that reside inside the unscoped level_enum inside the level namespace
-   `trace`
-   `debug`
-   `info`
-   `warn`
-   `error`
-   `crit`
