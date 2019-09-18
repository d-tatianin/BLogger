#include <BLogger/BLogger.h>

class test
{
public:
    int x1 = 99929;
    int x = 9999;

    friend std::ostream& operator<<(std::ostream& stream, const test& self)
    {
        return stream << "Serialized my test: " << self.x << self.x1;
    }
};


int main()
{
    // Create a logger with name "MyLogger" and a logging filter "Trace"
    blocking_logger logger("MyLogger", level::trace);
    logger.EnableTimestamps();
    logger.EnableConsoleLogger();
    logger.EnableColoredOutput();


    logger.Debug("My message is: {}, and also {}, {} + {}.", test(), 25, 26, test());

    std::cin.get();
    return 0;
}
