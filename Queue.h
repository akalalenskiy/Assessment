/// @brief Contains definition of QueueBase class, base class for queue
///
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

/// @brief Initial capacity of queue when it is created
static constexpr size_t InitialCapacity = 1000;
/// @brief Not used in this version. Intended to be used when a queue capacity shall be increased
static constexpr size_t IncreaseCapacity = 1000;


/// @brief Base class for queue
/// 
/// @tparam TKey Queue key type
/// 
/// @tparam TValue Contained value type
/// 
/// @tparam TQueueImpl Queue storage type
/// 
/// @tparam TConsumer Consumer (subscriber) type
/// 
template<typename TKey, typename TValue, typename TQueueImpl, typename TConsumer>
class QueueBase
{
    // These two friend classes for unot testing purposes
    friend class StringQueueBaseProxy;
    friend class IntQueueBaseProxy;

public:
    /// @brief Default constructor
    ///
    QueueBase() : m_key{} {}

    /// @brief Default constructor
    ///     Constructs a queue with given key and starts subscription thread.
    ///     Subscription thread remains suspended untill subscription
    /// @param key Queue key
    ///
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
   
    /// @brief Destructor.
    /// Stops subscription thread.
    ~QueueBase()
    {
        // Request watching thread to stop
        m_stopSubscriber = true;
        m_isNotEmpty.notify_one();

        if (m_thread.joinable())
            m_thread.join();
    }

    /// @brief Push an element to the queue
    /// 
    /// @param value A new element
    /// 
    void push(TValue value)
    {
        std::unique_lock<std::mutex> lk(m_lockForInsert);
        m_isNotFull.wait(lk,
            [this]() { return !m_container.isFull(); }
        );

        m_container.push(std::move(value));

        lk.unlock();
        m_isNotEmpty.notify_one();
    }

    /// @brief Get called by MultyQueueProcessor to initiate subscription
    /// 
    /// @<param consumer Weak pointer to subscriber
    /// 
    void subscribe(std::weak_ptr<TConsumer> consumer)
    {
        m_consumer = std::move(consumer);
        m_pauseSubscriber = false;
    }


protected:
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