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
```cpp

// 1. Create a logger using one of the factory functions

// Fully customizable
auto custom_logger = bl::Logger::Custom(
        "SomeTag",                   // logger tag
        bl::level::crit,             // log level filter
        bl::Logger::default_pattern, // logger pattern
        false,                       // is asynchronous
        bl::Sink::Stderr(true),      // any number of sinks at the end
        bl::Sink::File(
            "/logs",                 // path to a directory where you want the log files to be stored
            bl::infinite,            // bytes per file
            bl::infinite,            // maximum log files
            true                     // should rotate logs?
        )
    );

// Or maybe you want one of the preset loggers, like this one
auto async_logger = bl::Logger::AsyncConsole("my_async_logger", bl::level::info, true);

// 2. Start logging
async_logger->Warning("{0}, {}!", "Hello", "World");

// Even after a logger is created it's still highly customizable
// You can set a new tag
async_logger->SetTag("NewTag");

// Add a new sink
async_logger->AddSink(bl::Sink::Stdout(true));

// And even set a new pattern
async_logger->SetPattern("[{ts}][{tag}]\n[{lvl}] -> {msg}\n");
```
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

---
### - Logging your messages
-   `Log(level lvl, std::string message)` -> Logs the message with a given level.  
-   `Log(level lvl, std::string formattedMsg, Args ... args)` -> Logs the formatted message with a given level.

### - The following redundant member functions are also available with the same overloads as `Log()`, however, don't require a level argument
-   `Trace(...)` -> Logs the given message with logging level `trace`.
-   `Debug(...)`-> Logs the given message with logging level `debug`.
-   `Info(...)` -> Logs the given message with logging level `info`.
-   `Warning(...)` -> Logs the given message with logging level `warn`.
-   `Error(...)` -> Logs the given message with logging level `error`.
-   `Critical(...)` -> Logs the given message with logging level `crit`.

---
### - BLogger log message formatting
BLogger accepts the following formats:
-   `{}` a normal argument. Usage example: `logger->Critical("Something went wrong {}", error.message());`.
-   `{n}` a positional argument. Usage example: `logger->Info("{1}, {0}!", "World", "Hello")` -> prints `Hello, World!`.
-   You can also mix the two types like so `logger->Info("{}, {1}{0}", '!', "World", "Hello")` -> prints `Hello, World!`
  
Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::ostream`.

--- 
### - Unicode logging  
-   In order to enable Unicode mode type `#define BLOGGER_UNICODE_MODE` before including BLogger.h in any translation unit (aka .cpp).  
-   When Unicode mode is enabled, all functions expect wide strings and `const wchar_t*` for literals, wide string literal is defined by typing `L` before the opening quotes. Same way `std::ostream` is replaced with `std::wostream` for passing a user defined data type.
---
### - Misc member functions
-   `SetFilter(level lvl)` - > Sets the logging filter to the level specified.
-   `SetTag(std::string tag)` -> Sets the logger name to the name specified.
-   `Flush()` -> Flushes the logger.
-   `AddSink(Sink::Ptr sink)` -> Adds a sink to the logger.
-   `GlobalConsoleWriteLock()` -> returns the global mutex BLogger uses to write to a global sink. Use this mutex if you want to combine using BLogger with raw calls to `std::cout`. If you lock the mutex before writing to a global sink your message is guaranteed to be properly printed and be the default color.
-   `Formatter::CutIfExceeds(size_t size, std::string postfix)` -> Sets the maximum size of a log message. If the message exceeeds the set size it will be cut and the postfix will be inserted after. The postfix is set to `"..."` by default. Size can also be set to `bl::infinite`, which is the default setting.
-   `Formatter::SetTimestampFormat(std::string new_format)` -> Sets the timestamp format. Should be formatted according to the `strftime` specifications.
-   `Formatter::SetEnding(std::string ending)` -> Sets the global log message ending. Defaults to `\n`. The length is not included into message size calculations.
---
### - Logging sinks
BLogger offers a list or predefined sinks, which you can extend with ease.
-   `Sink::Stdout(bool colored)` -> a sink associated with `stdout`.
-   `Sink::Stderr(bool colored)` -> a sink associated with `stderr`.
-   `Sink::Console(bool colored)` -> same as `Sink::Stderr`.
-   `Sink::File(std::string directoryPath, size_t bytesPerFile, size_t maxLogFiles, bool rotateLogs)` -> a file sink.
