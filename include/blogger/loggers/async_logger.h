#pragma once

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>

#include <vector>
#include <deque>

#include <functional>
#include <memory>

#include "blogger/core.h"
#include "blogger/loggers/logger.h"
#include "blogger/sinks/file_sink.h"
#include "blogger/sinks/console_sink.h"
#include "blogger/sinks/colored_console_sink.h"
#include "blogger/log_levels.h"

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
        shared_sinks log_sinks;
    protected:
        bl_task(shared_sinks& sinks)
            : log_sinks(sinks)
        {
        }
    };

    class log_task : public bl_task
    {
    private:
        log_message msg;
    public:
        log_task(
            log_message&& msg,
            shared_sinks& sinks
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
        flush_task(shared_sinks& sinks)
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
        using task_ptr = std::unique_ptr<task>;
    private:
        std::vector<std::thread> m_pool;
        std::deque<task_ptr>     m_task_queue;
        std::mutex               m_queue_access;
        std::condition_variable  m_notifier;
        std::atomic_bool         m_running;
    private:
        thread_pool(uint16_t thread_count)
            : m_running(true)
        {
            m_pool.reserve(thread_count);

            for (uint16_t i = 0; i < thread_count; i++)
                m_pool.emplace_back([this]() { worker(); });
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

            while (m_running || did_work)
            {
                if (!did_work)
                    m_notifier.wait_for(
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
                locker lock(m_queue_access);

                if (!m_task_queue.empty())
                {
                    p = std::move(m_task_queue.front());
                    m_task_queue.pop_front();
                }
                else
                    return false;
            }

            p->complete();

            return true;
        }

        void shutdown()
        {
            m_running = false;
            m_notifier.notify_all();

            for (auto& worker : m_pool)
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
                locker lock(m_queue_access);

                if (m_task_queue.size() == BLOGGER_TASK_LIMIT)
                    m_task_queue.pop_front();

                m_task_queue.emplace_back(std::move(t));
            }

            m_notifier.notify_one();
        }

        ~thread_pool()
        {
            shutdown();
        }
    };

    class async_logger : public logger
    {
    public:
        async_logger(
            in_string tag,
            level lvl,
            bool default_pattern = true
        ): logger(tag, lvl, default_pattern)
        {
            thread_pool::get();
        }

        void flush() override
        {
            thread_pool::get().post_task(
                std::make_unique<flush_task>(m_sinks)
            );
        }

        ~async_logger() {}
    private:
        void post(log_message&& msg) override
        {
            thread_pool::get().post_task(
                std::make_unique<log_task>(std::move(msg), m_sinks)
            );
        }
    };
}

#undef BLOGGER_THREAD_COUNT
#undef BLOGGER_TASK_LIMIT
