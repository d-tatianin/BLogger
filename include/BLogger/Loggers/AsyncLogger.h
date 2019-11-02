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

#include "BLogger/Formatter/FormatUtilities.h"
#include "BLogger/Loggers/BaseLogger.h"
#include "BLogger/Sinks/FileSink.h"
#include "BLogger/Sinks/StdoutSink.h"
#include "BLogger/Sinks/ColoredStdoutSink.h"
#include "BLogger/LogLevels.h"

namespace BLogger {

    enum class task_type : uint16_t
    {
        flush = 1,
        log   = 2
    };

    class task
    {
    private:
        task_type t;
        BLoggerSharedSinkList log_sinks;
    protected:
        task(
            task_type t,
            BLoggerSharedSinkList& sinks
        ) : t(t),
            log_sinks(sinks)
        {
        }
    public:
        task_type type()
        {
            return t;
        }

        BLoggerSharedSinkList& sinks()
        {
            return log_sinks;
        }
    };

    class log_task : public task
    {
    private:
        BLoggerLogMessage msg;
    public:
        log_task(
            BLoggerLogMessage&& msg,
            BLoggerSharedSinkList& sinks
        ) : task(task_type::log, sinks),
            msg(std::move(msg))
        {
        }

        BLoggerLogMessage& message()
        {
            return msg;
        }
    };

    class flush_task : public task
    {
    public:
        flush_task(BLoggerSharedSinkList& sinks)
            : task(task_type::flush, sinks)
        {
        }
    };

    #define BLOGGER_TASK_LIMIT 10000

    class thread_pool
    {
    private:
        typedef std::shared_ptr<task>
            task_ptr;
        typedef std::lock_guard<std::mutex>
            locker;
        typedef std::unique_ptr<thread_pool>
            thread_pool_ptr;
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
                m_Pool.emplace_back(std::bind(&thread_pool::worker, this));
        }

        thread_pool(const thread_pool& other) = delete;
        thread_pool(thread_pool&& other) = delete;

        thread_pool& operator=(const thread_pool& other) = delete;
        thread_pool& operator=(thread_pool&& other) = delete;

        static std::mutex& HelperMutex()
        {
            static std::mutex* ihelper
                = new std::mutex();

            return *ihelper;
        }

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
                    p = m_TaskQueue.front();
                    m_TaskQueue.pop_front();
                }
                else
                    return false;
            }

            if (p->type() == task_type::log)
            {
                log_task* task = static_cast<log_task*>(p.get());

                task->message().finalize_format();

                for (auto& sink : *task->sinks())
                {
                    sink->write(task->message());
                }
            }
            else if (p->type() == task_type::flush)
            {
                flush_task* task = static_cast<flush_task*>(p.get());

                for (auto& sink : *task->sinks())
                {
                    sink->flush();
                }
            }
            return true;
        }

        void shutdown()
        {
            m_Running = false;
            m_Notifier.notify_all();

            for (auto& worker : m_Pool)
                worker.join();

            delete &HelperMutex();
        }

    public:
        static thread_pool_ptr& get()
        {
            static thread_pool_ptr instance;
            
            auto& helper = HelperMutex();
            locker lock(helper);

            if (!instance)
            {
                instance.reset(
                    new thread_pool(
                        std::thread::hardware_concurrency()
                    ));
            }

            return instance;
        }

        void post_message(BLoggerLogMessage&& message, BLoggerSharedSinkList& sinks)
        {
            {
                locker lock(m_QueueAccess);

                if (m_TaskQueue.size() == BLOGGER_TASK_LIMIT)
                    m_TaskQueue.pop_front();

                m_TaskQueue.emplace_back(new log_task(std::move(message), sinks));
            }

            m_Notifier.notify_one();
        }

        void post_flush(BLoggerSharedSinkList& sinks)
        {
            locker lock(m_QueueAccess);
            m_TaskQueue.emplace_back(new flush_task(sinks));
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
        )
            : BaseLogger(tag, lvl, default_pattern)
        {
        }

        void Flush() override
        {
            thread_pool::get()->post_flush(m_Sinks);
        }

        ~AsyncLogger() {}
    private:
        void Post(BLoggerLogMessage&& msg) override
        {
            thread_pool::get()->post_message(std::move(msg), m_Sinks);
        }
    };
}
