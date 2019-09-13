#include <BLogger/BLogger.h>

int main()
{
    // Create a logger with name "MyLogger" and a logging filter "Trace"
    BLogger logger("MyLogger", level::trace);

    // Enable logging to stdout(console)
    logger.EnableConsoleLogger();

    // Enable timestamps before the message
    logger.EnableTimestamps();

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
    logger.Trace("Trace with args %d", 25);

    // Log a message with log level 'Debug'
    logger.Debug("Debug message");
    logger.Debug("Debug with args %d", 25);

    // Flush the logger 
    // (primarily used to immediately dump all written data to file)
    logger.Flush();

    // Log a message with log level 'Information'
    logger.Info("Info message");
    logger.Info("Info with args %d", 25);

    // Set logging filter to 'Warning'
    // The logger will ignore all messages below 'Warning' log level
    logger.SetFilter(level::warn);

    // Don't append the name of the logger
    logger.DisableTag();

    // Log a message with log level 'Warning'
    logger.Warning("Warning message");
    logger.Warning("Warning with args %d", 25);

    // Log a message with log level 'Error'
    logger.Error("Error message");
    logger.Error("Error with args %d", 25);

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
