#pragma once

#include <boost/circular_buffer.hpp>

#include "Queue.h"

namespace MultyQueue_NS
{

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief Queue based on boost::circular_buffer
///     Implements WaitForSpace behavior
/// 
template<typename TValue>
class CircularQueue
{
    friend class StringQueueProxy;
    friend class IntQueueProxy;
public:
    CircularQueue() : m_container(InitialCapacity) {}

    static FullQueuePolicy fullQueuePolicy()
    {
        return FullQueuePolicy::WaitForSpace;
    }

    void push(TValue value)
    {
        m_container.push_back(std::move(value));
    }

    bool isFull() const
    {
        return  m_container.size() == m_container.capacity();
    }

    bool isEmpty() const
    {
        return m_container.size() == 0;
    }

    TValue pop()
    {
        TValue res(std::move(*m_container.begin()));
        m_container.pop_front();

        return res;
    }

    bool isInited() const
    {
        return true;
    }

private:
    boost::circular_buffer<TValue> m_container;
};

}
