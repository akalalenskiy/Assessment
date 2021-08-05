#pragma once

#include <utility>

namespace MultyQueue_NS
{

template<typename TKey, typename TValue, typename TConsumer>
class ConsumerBase
{
public:
    void Consume(TKey key, TValue value)
    {
        return static_cast<TConsumer*>(this)->Consume(key, value);
    }
};

}