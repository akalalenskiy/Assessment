#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "Common.h"

// Forward declaration
class StringQueueProxy;
class IntQueueProxy;
class StringQueueBaseProxy;
class IntQueueBaseProxy;

namespace MultyQueue_NS
{

static constexpr size_t InitialCapacity = 1000;
static constexpr size_t IncreaseCapacity = 1000;

template<typename TKey, typename TValue, typename TQueueImpl, typename TConsumer>
class QueueBase
{
    friend class StringQueueBaseProxy;
    friend class IntQueueBaseProxy;

public:
    QueueBase() : m_key{} {}

    QueueBase(const TKey& key) : m_key(key)
    {
        auto watchingLambda = [this]() {
            while (!m_stopSubscriber)
            {
                std::unique_lock lk(m_lockForRead);
                m_isNotEmpty.wait(lk,
                    [this]() { return m_stopSubscriber || (!m_container.isEmpty() && !m_pauseSubscriber); }
                );

                if (m_stopSubscriber)
                    return;

                if (auto consumer = m_consumer.lock())
                {
                    consumer->consume(m_key, std::move(m_container.pop()));
                }
                else
                {
                    m_pauseSubscriber = true;
                }

                lk.unlock();
                m_isNotFull.notify_one();
            }
        };

        // start watching thread (paused)
        m_thread = std::thread(std::move(watchingLambda));
    }
   
    ~QueueBase()
    {
        // Request watching thread to stop
        m_stopSubscriber = true;
        m_isNotEmpty.notify_one();

        if (m_thread.joinable())
            m_thread.join();
    }

    void push(TValue value)
    {
        std::unique_lock<std::mutex> lk(m_lockForInsert);
        m_isNotFull.wait(lk,
            [this]() { return !m_container.isFull(); /*!static_cast<TQueueImpl*>(this)->isFull();*/ }
        );

        m_container.push(std::move(value));//static_cast<TQueueImpl*>(this)->push(std::move(value));

        lk.unlock();
        m_isNotEmpty.notify_one();
    }

    void subscribe(std::weak_ptr<TConsumer> consumer)
    {
        m_consumer = std::move(consumer);
        m_pauseSubscriber = false;
    }

    bool isInited() const
    {
        return true;
    }

protected:
    void run()
    {

    }

    const TKey m_key;

    TQueueImpl m_container;

    std::atomic_bool m_isInited = false;

    /// @brirf Signal that buffer is not full
    std::condition_variable m_isNotFull;
    std::mutex m_lockForInsert;

    /// @brirf Signal that buffer is not empty
    std::condition_variable m_isNotEmpty;
    std::mutex m_lockForRead;

    std::weak_ptr<TConsumer> m_consumer;

    std::atomic_bool m_pauseSubscriber = true;
    std::atomic_bool m_stopSubscriber = false;

    std::thread m_thread;
};

}