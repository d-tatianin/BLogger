#include <blogger/blogger.h>

class MySerializableClass
{
public:
    int x = 66;
    int y = 25;

    friend std::ostream& operator<<(std::ostream& stream, MySerializableClass& mc)
    {
        return stream << "MyValues: " << mc.x << ", " << mc.y;
    }
};

int main()
{
    // -------- Creating a simple logger and logging a message

    // Create a console logger
    auto logger = bl::Logger::Console("tag", bl::level::warn, true);

    // Log a 'Warning message'
    logger->Warning("A warning log message{}", '!');

    // --------------------------------------------------



    // -------- Manually changing the color of the console streams

    // You can manually change the console stream color
    // by using bl::color::some_color
    std::cout << bl::color::cyan    << "This is cyan\n"
              << bl::color::magenta << "This is magenta\n"
              << bl::color::reset   << "This is normal\n";

    // --------------------------------------------------



    // -------- Creating an asynchronous logger and setting a custom pattern

    // Create an asynchronous console logger
    auto async_logger = bl::Logger::AsyncConsole("my_async_logger", bl::level::info, true);

    // Set the logging pattern of our new logger
    // {ts}  -> timestamp
    // {lvl} -> log level of the message
    // {tag} -> current logger tag(name)
    // {msg} -> the message itself
    async_logger->SetPattern("[{ts}][{tag}]\n[{lvl}] -> {msg}\n");

    // A class that can be serialized
    // by the logger, because it has the
    // operator<< overloaded for std::ostream
    MySerializableClass mc;

    // Log an error with multiple arguments
    // Notice how we mix positional/non-positional args here
    async_logger->Error("An error message, {0}! {}", "something bad happened", mc);

    // --------------------------------------------------



    // -------- Creating a custom logger with sinks of your choice

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

    // --------------------------------------------------

    // Will only shutdown after AsyncLogger
    // has finished all of its tasks
    return 0;
}
