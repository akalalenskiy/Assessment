#pragma once

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <boost/circular_buffer.hpp>

#include "Common.h"

namespace MultyQueue_NS
{

template<typename TValue, typename TQueueImpl>
class QueueBase
{
public:
    static constexpr size_t InitialCapacity = 1000;
    static constexpr size_t IncreaseCapacity = 1000;

public:
    QueueBase()
    {
    }

    void init()
    {
        m_isInited = true;
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

    void pop(TValue res)
    {
        std::unique_lock lk(m_lockForRead);
        m_isNotEmpty.wait(lk,
            [this]() { return !m_container.isEmpty(); /*!static_cast<TQueueImpl*>(this)->isEmpty();*/ }
        );

        res = std::move( m_container.pop() /*static_cast<TQueueImpl*>(this)->pop()*/);

        lk.unlock();
        m_isNotFull.notify_one();
    }

    bool isInited() const
    {
        return true;
    }

protected:
    TQueueImpl m_container;

    std::atomic_bool m_isInited = false;

    /// @brirf Signal that buffer is not full
    std::condition_variable m_isNotFull;
    std::mutex m_lockForInsert;

    /// @brirf Signal that buffer is not empty
    std::condition_variable m_isNotEmpty;
    std::mutex m_lockForRead;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////
/// 
/// @brief Queue based on boost::circular_buffer
///     Implements WaitForSpace behavior
/// 
template<typename TValue>
class CircularQueue
{
public:
    CircularQueue() : m_container(QueueBase<TValue, CircularQueue>::InitialCapacity) {}

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
        const auto size = m_container.size();
        const auto capacity = m_container.capacity();
        return  size == capacity ;
    }

    bool isEmpty() const
    {
        return m_container.size() == 0;
    }

    TValue pop()
    {
        TValue res(std::move(*m_container.begin()));
        m_container.pop_front();
    }

    bool isInited() const
    {
        return true;
    }

private:
    boost::circular_buffer<TValue> m_container;
};

}