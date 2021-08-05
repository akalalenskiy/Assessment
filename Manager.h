#pragma once

#include <unordered_map>

#include "ConsumerBase.h"
#include "Queue.h"

namespace MultyQueue_NS
{

template<typename TKey, typename TValue/*, typename TStorage*/, typename TConsumer, typename THash = std::hash<TKey>>
class Manager
{
public:
    void Subscribe(ConsumerBase<TKey, TValue, TConsumer>* consumer)
    {
        m_consumers["0"] = consumer;
    }

    void start()
    {
        
    }

    void Enqueue(TKey key, TValue value)
    {
        auto* consumer = m_consumers["0"];
        consumer->Consume(key, value);
    }

private:
    std::unordered_map<TKey, TValue, THash> m_queues;
    std::unordered_map<TKey, ConsumerBase<TKey, TValue, TConsumer>*, THash> m_consumers;
};

}