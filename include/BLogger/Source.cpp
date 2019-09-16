#include "BLogger.h"
#include "Timer.h"

#include <thread>
#include <future>
#include <condition_variable>
#include <vector>


//std::mutex mu;
//std::condition_variable vb;
//
//
//void thread()
//{
//    
//    std::unique_lock<std::mutex> ul(mu);
//
//    while (true)
//    {
//            vb.wait(ul);
//
//            std::cout << "woke up" << std::endl;
//            Sleep(500);
//    }
//}
//
//int main()
//{
//    std::thread t1(thread);
//
//    Sleep(1000);
//    vb.notify_one();
//    vb.notify_one();
//    vb.notify_one();
//
//    t1.join();
//    std::cin.get();
//    return 0;
//}

//int main() 
//{
//    BLoggerBlock logger("CORE");
//    logger.EnableTimestamps();
//    logger.EnableConsoleLogger();
//    std::vector<double> elapsed(1000000);
//    
//
//    Timer t("asd", TimeUnit::MICROSECONDS);
//
//    for(int i =0; i < 1000000; i++)
//    {
//        t.Reset();
//        logger.Warning("Crit msg");
//        elapsed.emplace_back(t.GetElapsed());
//    }
//
//    double total = 0;
//    for (auto& d : elapsed)
//    {
//        total += d;
//    }
//    total /= elapsed.size();
//
//    std::cout << "ELAPSED: " << total << std::endl;
//
//    std::cin.get();
//}

int main()
{
    BLoggerBlock logger("Test", level::trace);
    logger.EnableConsoleLogger();
    logger.EnableColoredOutput();
    logger.EnableTimestamps();

    logger.InitFileLogger("log", 315, 3, false);
    logger.EnableFileLogger();

    for (int i = 0; i < 1000; i++)
    {
        logger.Critical("Some critical message with args %d", 25);
        logger.Critical("Some critical message with args");
    }

    std::cin.get();
    return 0;
}