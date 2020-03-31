#include <blogger/blogger.h>

class MySerializableClass
{
public:
    int x = 66;
    int y = 25;

    friend std::ostream& operator<<(std::ostream& stream, MySerializableClass& mc)
    {
        return stream << "Here are my important values: " << mc.x << ", " << mc.y;
    }
};

int main()
{
    // BLogger properties struct
    // used to customize the logger
    // before creating it
    bl::Props props;

    // Creates an Async logger if true
    // blocking otherwise
    props.async = true;

    // Enables the stdout sink
    props.console_logger = true;

    // Enable colored messages for the stdout sink
    // The default colors for each level are defined in LogLevels.h
    // Feel free to change the default colors however you want
    // You can find all available colors in Colors.h
    props.colored = true;

    // Sets the logger tag
    // (can be changed later)
    props.tag = "MyLogger";

    // If the pattern is empty
    // a default one will be set
    // by BLogger upon creation
    props.pattern = "";

    // Sets the logging filter
    props.filter = level::trace;

    // Enables the file logger
    props.file_logger = true;

    // Path to a directory
    // where you want the log files
    // to be stored
    props.path = "logs";

    // Maximum bytes per log file
    props.bytes_per_file = BLOGGER_INFINITE;

    // Maximum log files
    props.log_files = 1;

    // Overwrites the oldest
    // log file if the limit is hit
    props.rotate_logs = false;

    // Use the properties with BLogger factory
    // returns a pointer to the new logger
    auto logger = bl::Logger::CreateFromProps(props);

    // Set the pattern of the logger
    // {ts}  -> timestamp
    // {lvl} -> log level of the message
    // {tag} -> current logger tag(name)
    // {msg} -> the message itself
    logger->SetPattern("[{ts}][{tag}]\n[{lvl}] -> {msg}\n");

    // Changes the name of the logger
    logger->SetTag("NewFancyName");

    // Log a message with log level 'Trace'
    logger->Trace("Trace message");

    // Use a positional argument
    logger->Trace("Trace with args {0}", 25);

    // Log a message with log level 'Debug'
    logger->Debug("Debug message");

    // Mix positional / non positional arguments
    logger->Debug("{1} with args {}", "hello", "Debug");

    // Flush the logger 
    // (primarily used to immediately dump all written data to file)
    logger->Flush();

    // Log a message with log level 'Information'
    logger->Info("Info message");
    logger->Info("Info with args {}", 25.0f);

    // Set logging filter to 'Warning'
    // The logger will ignore all messages below 'Warning' log level
    logger->SetFilter(level::warn);

    // Log a message with log level 'Warning'
    logger->Warning("Warning message");
    logger->Warning("Warning with args {}", 25);

    // Log a message with log level 'Error'
    logger->Error("Error message");

    // passing an object of our class
    // which has an overload for ostream operator '<<'
    MySerializableClass mc;
    logger->Error("Error with user defined args ({})", mc);

    // Log a message with log level 'Critical'
    logger->Critical("Critical message");
    logger->Critical("Critical with args {}", 25);

    // You can also use factory without BLoggerProps
    // for quickly creating a logger with predefined sinks
    auto blockingLogger = bl::Logger::CreateBlockingConsole("BlockingLogger", level::trace);

    std::cin.get();
    // Will only shutdown after AsyncLogger
    // has finished all of its tasks
    return 0;
}
