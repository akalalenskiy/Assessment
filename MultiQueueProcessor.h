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
/// @tparam TConsumer Consumer type
/// 

template<typename TKey, typename TValue, typename TQueue, typename TConsumer, typename TKeyHash = std::hash<TKey>>
class MultiQueueProcessor
{
    friend class MultyQueueProcessorProxy;
public:
    void Subscribe(TKey key, std::shared_ptr<TConsumer>& consumer)
    {
        getQueue(key)->subscribe(std::weak_ptr(consumer));
    }

    void Enqueue(const TKey& key, TValue value)
    {
        getQueue(key)->push(value);
    }

private:
    using Queue = QueueBase<TKey, TValue, TQueue, TConsumer>;
    using QueuePtr = std::shared_ptr<Queue>;

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