#include <BLogger/BLogger.h>

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
    // Create a blocking logger with name "MyLogger" 
    // and a logging filter "Trace"
    BlockingLogger logger("MyLogger", level::trace);

    // Set the pattern of the logger
    // {ts}  -> timestamp
    // {lvl} -> log level of the message
    // {tag} -> current logger tag(name)
    // {msg} -> the message itself
    logger.SetPattern("[{ts}][{lvl}][{tag}] {msg}");

    // Enable logging to stdout(console)
    logger.EnableConsoleLogger();

    // Enable colored messages for stdout
    // The default colors for each level are defined in BLogger.h
    // Feel free to change the default colors however you want
    // You can find all availble colors in Colors.h
    logger.EnableColoredOutput();

    // We have to initialize the file logger before enabling it
    // 1st parameter -> The directory where you want you logging files to be stored
    // 2nd parameter -> Bytes per file limit (infinite in our case)
    // 3rd parameter -> Maximum number of files
    // 4th parameter (default is true) -> Enable log rotation
    // Log rotation means the oldest file gets overwritten once the file limit is hit
    logger.InitFileLogger("dev/mylogs", BLOGGER_INFINITE, 2, true);
    logger.EnableFileLogger();

    // Change the name of the logger
    logger.SetTag("NewFancyName");

    // Log a message with log level 'Trace'
    logger.Trace("Trace message");

    // Use a positional argument
    logger.Trace("Trace with args {0}", 25);

    // Log a message with log level 'Debug'
    logger.Debug("Debug message");

    // Mix positional / none positional arguments
    logger.Debug("{1} with args {}", "hello", "Debug");

    // Flush the logger 
    // (primarily used to immediately dump all written data to file)
    logger.Flush();

    // Log a message with log level 'Information'
    logger.Info("Info message");
    logger.Info("Info with args {}", 25.0f);

    // Set logging filter to 'Warning'
    // The logger will ignore all messages below 'Warning' log level
    logger.SetFilter(level::warn);

    // Log a message with log level 'Warning'
    logger.Warning("Warning message");
    logger.Warning("Warning with args {}", 25);

    // Log a message with log level 'Error'
    logger.Error("Error message");

    // passing an object of our class
    // which has an overload for ostream operator '<<'
    MySerializableClass mc;
    logger.Error("Error with user defined args ({})", mc);

    // Log a message with log level 'Critical'
    logger.Critical("Critical message");
    logger.Critical("Critical with args {}", 25);

    // Temporarily disable the file logger
    // Can be reactvated any time with 'EnableFileLogger()'
    logger.DisableFileLogger();

    // Permanently disable the file logger
    // If you want to reactivate it, you will have to call
    // 'InitFileLogger()' again
    logger.TerminateFileLogger();

    // Disable logging to stdout(console)
    logger.DisableConsoleLogger();

    std::cin.get();
    return 0;
}
