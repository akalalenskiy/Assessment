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
class Manager
{
public:
    using QueueName = std::string;

    template<typename TKey, typename TValue>
    static void createQueue()
    {

    }

    template<typename TKey, typename TValue>
    static void createNamedQueue(const QueueName& queueName)
    {

    }

    void Subscribe(TKey key, std::shared_ptr<TConsumer>& consumer)
    {
        m_consumers[key] = consumer;
    }

    void start()
    {
        
    }

    void Enqueue(TKey key, TValue value)
    {
        auto queue = m_queues[key];
        if (!queue)
        {
            queue = std::make_shared<QueueBase<TValue, TQueue>>();
            m_queues[key] = queue;
        }

        queue->push(value);
    }

private:
    std::unordered_map<TKey, std::shared_ptr<QueueBase<TValue, TQueue>>, TKeyHash> m_queues;
    std::unordered_map<TKey, std::weak_ptr<TConsumer>, TKeyHash> m_consumers;
};

}