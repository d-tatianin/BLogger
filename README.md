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

// You can use one of the common preconfigured loggers
auto async_logger = bl::logger::make_async_console();

// Or maybe you want a fully customizable one
auto custom_logger = bl::logger::make_custom(
        "SomeTag",                   // logger tag
        bl::level::crit,             // log level filter
        bl::logger::default_pattern, // logger pattern
        false,                       // is asynchronous
        bl::sink::make_stderr(),     // any number of sinks at the end
        bl::sink::make_file(
            "/logs",                 // path to a directory where you want the log files to be stored
            bl::infinite,            // bytes per file
            bl::infinite,            // maximum log files
            true                     // should rotate logs?
        )
    );

// 2. Start logging
async_logger->warning("{0}, {}!", "Hello", "World");

// Even after a logger is created it's still highly customizable:

// You can set a new tag
async_logger->set_tag("NewTag");

// Add a new sink
async_logger->add_sink(bl::sink::make_stdout());

// And even set a new pattern
async_logger->set_pattern("[{ts}][{tag}]\n[{lvl}] -> {msg}\n");
```
---
### - Available factory functions
```cpp
// A basic console logger
bl::logger::make_console(string tag, level lvl, string pattern, bool colored);

// An asychronous console logger
bl::logger::make_async_console(string tag, level lvl, string pattern, bool colored);

// A basic file logger
bl::logger::make_file(string tag, 
                      level lvl, 
                      string pattern,
                      string directory_path,
                      size_t byte_per_file,
                      size_t max_log_files,
                      bool rotate_logs);

// An asynchronous file logger
bl::logger::make_async_file(string tag, 
                            level lvl, 
                            string pattern,
                            string directory_path,
                            size_t byte_per_file,
                            size_t max_log_files,
                            bool rotate_logs);

// Or a fully customizable logger with
// any number of sinks at the end
bl::logger::make_custom(string tag,
                        level lvl,
                        string pattern,
                        bool asynchronous,
                        Sinks... sinks);
```

---
### - Setting the pattern  
Arguments you can use for creating a custom pattern:
-   `{ts}` -> timestamp.
-   `{lvl}` -> logging level of the current message.
-   `{tag}` -> logger tag(name).
-   `{msg}` -> the message itself.  

After you've decided on your pattern you can set it by calling `set_pattern(string pattern)`.

Here's an example of an interesting pattern `"[{ts}][{tag}]\n[{lvl}] -> {msg}\n"`, which looks like this:
![alt-text](https://i.ibb.co/w0yfBcL/BLogger.png)

---
### - Logging your messages
-   `log(level lvl, string message)` -> Logs the message with a given level.  
-   `log(level lvl, string formattedMsg, Args ... args)` -> Logs the formatted message with a given level.

### - The following redundant member functions are also available with the same overloads as `log()`, however, don't require a level argument
-   `trace(...)` -> Logs the given message with logging level `trace`.
-   `debug(...)`-> Logs the given message with logging level `debug`.
-   `info(...)` -> Logs the given message with logging level `info`.
-   `warning(...)` -> Logs the given message with logging level `warn`.
-   `error(...)` -> Logs the given message with logging level `error`.
-   `critical(...)` -> Logs the given message with logging level `crit`.

---
### - BLogger log message formatting
BLogger accepts the following formats:
-   `{}` a normal argument. Usage example: `logger->critical("Something went wrong {}", error.message());`.
-   `{n}` a positional argument. Usage example: `logger->info("{1}, {0}!", "World", "Hello")` -> prints `Hello, World!`.
-   You can also mix the two types like so `logger->info("{}, {1}{0}", '!', "World", "Hello")` -> prints `Hello, World!`
  
Note: if you are passing a user defined data type make sure it has the `<<` operator overloads for `std::ostream`.

--- 
### - Unicode logging  
-   In order to enable Unicode mode type `#define BLOGGER_UNICODE_MODE` before including BLogger.h in any translation unit (aka .cpp).  
-   When Unicode mode is enabled, all functions expect wide strings and `const wchar_t*` for literals, wide string literal is defined by typing `L` before the opening quotes. Same way `std::ostream` is replaced with `std::wostream` for passing a user defined data type.
---
### - Misc member functions
-   `set_filter(level lvl)` - > Sets the logging filter to the level specified.
-   `set_tag(string tag)` -> Sets the logger name to the name specified.
-   `flush()` -> Flushes the logger.
-   `add_sink(sink::ptr sink)` -> Adds a sink to the logger.
-   `global_console_write_lock()` -> returns the global mutex BLogger uses to write to a global sink. Use this mutex if you want to combine using BLogger with raw calls to `std::cout`. If you lock the mutex before writing to a global sink your message is guaranteed to be properly printed and be the default color.
-   `formatter::cut_if_exceeds(size_t size, string postfix)` -> Sets the maximum size of a log message. If the message exceeeds the set size it will be cut and the postfix will be inserted after. The postfix is set to `"..."` by default. Size can also be set to `bl::infinite`, which is the default setting.
-   `formatter::set_timestamp_format(string new_format)` -> Sets the timestamp format. Should be formatted according to the `strftime` specifications.
-   `formatter::set_ending(string ending)` -> Sets the global log message ending. Defaults to `\n`. The length is not included into message size calculations.
---
### - Logging sinks
BLogger offers a list or predefined sinks, which you can extend with ease.
-   `sink::make_stdout(bool colored)` -> a sink associated with `stdout`.
-   `sink::make_stderr(bool colored)` -> a sink associated with `stderr`.
-   `sink::make_stdlog(bool colored)` -> a wrapper around `std::clog` (uses `stderr`).
-   `sink::make_console(bool colored)` -> same as `sink::make_stdlog`.
-   `sink::make_file(string directoryPath, size_t bytesPerFile, size_t maxLogFiles, bool rotateLogs)` -> a file sink.
