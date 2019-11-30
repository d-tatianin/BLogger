#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <vector>
#include <deque>
#include <unordered_map>

#include <functional>
#include <memory>

#include "BLogger/Core.h"
#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/Sinks/FileSink.h"
#include "BLogger/Sinks/StdoutSink.h"
#include "BLogger/Sinks/ColoredStdoutSink.h"
#include "BLogger/LogLevels.h"

namespace BLogger {

    class task
    {
    protected:
        BLoggerSharedSinks log_sinks;
    protected:
        task(BLoggerSharedSinks& sinks)
            : log_sinks(sinks)
        {
        }
    public:
        virtual void complete() = 0;
    };

    class log_task : public task
    {
    private:
        BLoggerLogMessage msg;
    public:
        log_task(
            BLoggerLogMessage&& msg,
            BLoggerSharedSinks& sinks
        ) : task(sinks),
            msg(std::move(msg))
        {
        }

        void complete() override
        {
            msg.finalize_format();

            for (auto& sink : *log_sinks)
            {
                sink->write(msg);
            }
        }
    };

    class flush_task : public task
    {
    public:
        flush_task(BLoggerSharedSinks& sinks)
            : task(sinks)
        {
        }

        void complete() override
        {
            for (auto& sink : *log_sinks)
            {
                sink->flush();
            }
        }
    };

    #define BLOGGER_TASK_LIMIT 10000
    
    class thread_pool
    {
    public:
        typedef std::unique_ptr<task>
            task_ptr;
        typedef std::lock_guard<std::mutex>
            locker;
    private:
        std::vector<std::thread> m_Pool;
        std::deque<task_ptr>     m_TaskQueue;
        std::mutex               m_QueueAccess;
        std::condition_variable  m_Notifier;
        bool                     m_Running;
    private:
        thread_pool(uint16_t thread_count)
            : m_Running(true)
        {
            m_Pool.reserve(thread_count);

            for (uint16_t i = 0; i < thread_count; i++)
                m_Pool.emplace_back([this]() { worker(); });
        }

        thread_pool(const thread_pool& other) = delete;
        thread_pool(thread_pool&& other) = delete;

        thread_pool& operator=(const thread_pool& other) = delete;
        thread_pool& operator=(thread_pool&& other) = delete;

        void worker()
        {
            bool did_work = true;
            std::mutex worker_lock;
            std::unique_lock<std::mutex> task_waiter(worker_lock);

            while (m_Running || did_work)
            {
                if (!did_work)
                    m_Notifier.wait_for(
                        task_waiter,
                        std::chrono::seconds(5)
                    );

                did_work = do_work();
            }
        }

        bool do_work()
        {
            task_ptr p;

            {
                locker lock(m_QueueAccess);

                if (!m_TaskQueue.empty())
                {
                    p = std::move(m_TaskQueue.front());
                    m_TaskQueue.pop_front();
                }
                else
                    return false;
            }

            p->complete();

            return true;
        }

        void shutdown()
        {
            m_Running = false;
            m_Notifier.notify_all();

            for (auto& worker : m_Pool)
                worker.join();
        }

    public:
        static thread_pool& get()
        {
            static thread_pool instance(
                std::thread::hardware_concurrency()
            );

            return instance;
        }

        void post_task(std::unique_ptr<task> t)
        {
            {
                locker lock(m_QueueAccess);

                if (m_TaskQueue.size() == BLOGGER_TASK_LIMIT)
                    m_TaskQueue.pop_front();

                m_TaskQueue.emplace_back(std::move(t));
            }

            m_Notifier.notify_one();
        }

        ~thread_pool()
        {
            shutdown();
        }
    };

    class AsyncLogger : public BaseLogger
    {
    public:
        AsyncLogger(
            BLoggerInString tag,
            level lvl,
            bool default_pattern = true
        ): BaseLogger(tag, lvl, default_pattern)
        {
            //thread_pool::get();
        }

        void Flush() override
        {
            thread_pool::get().post_task(
                std::make_unique<flush_task>(m_Sinks)
            );
        }

        ~AsyncLogger() {}
    private:
        void Post(BLoggerLogMessage&& msg) override
        {
            thread_pool::get().post_task(
                std::make_unique<log_task>(std::move(msg), m_Sinks)
            );
        }
    };
}
