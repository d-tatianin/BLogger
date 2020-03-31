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

#include "blogger/Core.h"
#include "blogger/Loggers/Logger.h"
#include "blogger/Sinks/FileSink.h"
#include "blogger/Sinks/StdoutSink.h"
#include "blogger/Sinks/ColoredStdoutSink.h"
#include "blogger/LogLevels.h"

// Probably shouldnt be higher than 4 because the thread_pool
// threads will spend most of the time waiting
// for the I/O mutex anyway. Unless you're posting your own tasks.
#define BLOGGER_THREAD_COUNT std::thread::hardware_concurrency()

#define BLOGGER_TASK_LIMIT 10000

namespace bl {

    // You can make your own tasks
    // and feed them to the thread_pool
    // as well.
    class task
    {
    public:
        virtual void complete() = 0;

        virtual ~task() = default;
    };

    class bl_task : public task
    {
    protected:
        SharedSinks log_sinks;
    protected:
        bl_task(SharedSinks& sinks)
            : log_sinks(sinks)
        {
        }
    };

    class log_task : public bl_task
    {
    private:
        BLoggerLogMessage msg;
    public:
        log_task(
            BLoggerLogMessage&& msg,
            SharedSinks& sinks
        ) : bl_task(sinks),
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

    class flush_task : public bl_task
    {
    public:
        flush_task(SharedSinks& sinks)
            : bl_task(sinks)
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
        std::atomic_bool         m_Running;
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
                BLOGGER_THREAD_COUNT
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

    class AsyncLogger : public Logger
    {
    public:
        AsyncLogger(
            InString tag,
            level lvl,
            bool default_pattern = true
        ): Logger(tag, lvl, default_pattern)
        {
            thread_pool::get();
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
