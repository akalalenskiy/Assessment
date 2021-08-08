#pragma once

#include <utility>

namespace MultyQueue_NS
{

/// @brief Base class for consumers (subscribers)
/// 
/// @tparam TKey Queue key type
/// 
/// @tparam TValue Type of queuelement
/// 
/// @tparam TConsumer Type of consumer, derived from this class (CRTP pattern used)
/// 
template<typename TKey, typename TValue, typename TConsumer>
class ConsumerBase
{
public:
    void consume(TKey key, TValue value)
    {
        return static_cast<TConsumer*>(this)->consume(key, value);
    }
};

}