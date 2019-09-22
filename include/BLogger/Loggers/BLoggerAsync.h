#pragma once

#include <functional>
#include <memory>
#include <vector>
#include <deque>
#include <thread>
#include <mutex>

#include "BLogger/OS/Colors.h"

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
    protected:
        task(task_type t)
            : t(t)
        {
        }
    public:
        task_type type()
        {
            return t;
        }
    };

    class log_task : public task
    {
    private:
        LogMsg msg;
    public:
        log_task(LogMsg&& msg)
            : task(task_type::log),
            msg(msg)
        {
        }

        LogMsg& message()
        {
            return msg;
        }
    };

    class flush_task : public task
    {
    public:
        flush_task()
            : task(task_type::flush)
        {
        }
    };

    #define BLOGGER_TASK_LIMIT 100

    class thread_pool
    {
    private:
        typedef std::shared_ptr<task>
            task_ptr;
        typedef std::lock_guard<std::mutex>
            locker;

        static thread_pool* instance;
        std::vector<std::thread> m_Pool;
        std::deque<task_ptr> m_TaskQueue;
        std::mutex m_QueueAccess;
        std::mutex m_GlobalWrite;
        bool m_Running;

        thread_pool(uint16_t thread_count)
            : m_Running(true)
        {
            m_Pool.reserve(std::thread::hardware_concurrency());

            for (uint16_t i = 0; i < thread_count; i++)
                m_Pool.emplace_back(std::bind(&thread_pool::worker, this));
        }

        void worker()
        {
            bool did_work = do_work();

            while (m_Running || did_work)
            {
                if (!did_work)
                    std::this_thread::sleep_for(std::chrono::seconds(1));

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

                {
                    locker lock(m_GlobalWrite);

                    std::cout.write(task->message().data(), task->message().size());
                }
            }
            else if (p->type() == task_type::flush)
            {
                {
                    locker lock(m_GlobalWrite);

                    std::cout.flush();
                }
            }

            return true;
        }

    public:
        static thread_pool* get()
        {
            if (!instance)
            {
                instance = new thread_pool(std::thread::hardware_concurrency());
            }

            return instance;
        }

        void post(LogMsg&& message)
        {
            {
                locker lock(m_QueueAccess);

                if (m_TaskQueue.size() == BLOGGER_TASK_LIMIT)
                    m_TaskQueue.pop_front();

                m_TaskQueue.emplace_back(new log_task(std::move(message)));
            }
        }

        void flush()
        {
            locker lock(m_QueueAccess);
            m_TaskQueue.emplace_back(new flush_task());
        }

        void shutdown()
        {
           if (m_Running)
           {
               m_Running = false;

               for (auto& worker : m_Pool)
                   worker.join();
           }
        }
    };

    thread_pool* thread_pool::instance;

    class BLoggerAsync : public BLoggerBase
    {
    public:
        BLoggerAsync()
            : BLoggerBase()
        {
        }

        BLoggerAsync(const std::string& tag)
            : BLoggerBase(tag)
        {
        }

        BLoggerAsync(
            const std::string& tag,
            level lvl,
            bool default_pattern = false
        )
            : BLoggerBase(tag, lvl, default_pattern)
        {
        }

        ~BLoggerAsync()
        {
            thread_pool::get()->shutdown();
        }

    private:
        void Post(LogMsg&& msg) override
        {
            thread_pool::get()->post(std::move(msg));
        }

        void Flush() override
        {
            thread_pool::get()->flush();
        }
    };
}