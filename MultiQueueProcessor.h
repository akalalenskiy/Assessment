/// 
/// @brief One queue per each key + TValue
/// 

#pragma once

#include <memory>
#include <unordered_map>

#include "ConsumerBase.h"
#include "Queue.h"

namespace MultyQueue_NS
{

/// @brief Queue controller (processor) class.
///        One instance of processor per each combination of TKey and TValue
/// 
/// @tparam TKey Key type. Processor maintains one queue per each value of TKey
/// 
/// @tparam TValue Value type
/// 
/// @tparam TQueue Queue type
/// 
/// @tparam TConsumer Consumer type
/// 
/// @tparam TKeyHash Hash function for TKey
/// 

template<typename TKey, typename TValue, typename TQueue, typename TConsumer, typename TKeyHash = std::hash<TKey>>
class MultiQueueProcessor
{
    // For unit testing purposes
    friend class MultyQueueProcessorProxy;
public:
    /// @brief Subscriber (consumer) shall use this method to subscribe for new objects in queue
    /// 
    /// @param key Key. Subscriber gets new objects addet to the queue with this key
    /// 
    /// @param consumer Shared pointer to consumer
    void subscribe(TKey key, std::shared_ptr<TConsumer>& consumer)
    {
        getQueue(key)->subscribe(std::weak_ptr(consumer));
    }

    /// @brief Put an element with given key and value into queue
    /// 
    /// @param key New elements key. Identifies the queue instances where to add the element
    /// 
    /// @param value New elements value
    void enqueue(const TKey& key, TValue value)
    {
        getQueue(key)->push(value);
    }

private:
    using Queue = QueueBase<TKey, TValue, TQueue, TConsumer>;
    using QueuePtr = std::shared_ptr<Queue>;

    // Auxiliary function returns a queue instance if exists or creates a new one
    QueuePtr getQueue(const TKey& key)
    {
        auto queue = m_queues[key];
        if (!queue)
        {
            queue = std::make_shared<Queue>(key);
            m_queues[key] = queue;
        }

        return queue;
    }

    std::unordered_map<TKey, QueuePtr, TKeyHash> m_queues;
};

}