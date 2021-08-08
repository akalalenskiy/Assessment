#include "pch.h"

#include "../CircularQueue.h"
#include "../ConsumerBase.h"
#include "../MultiQueueProcessor.h"
#include "../Queue.h"

#include <chrono>
#include <iostream>
#include <string>
#include <set>
#include <thread>

using namespace std::chrono_literals;

using namespace MultyQueue_NS;

class StringConsumer : public ConsumerBase<std::string, std::string, StringConsumer>
{
public:
    void consume(const std::string&, const std::string&)
    {
        // do nothing here
    }
};

class IntConsumer : public ConsumerBase<size_t, size_t, IntConsumer>
{
public:
    IntConsumer(std::set<size_t>& val) : m_controlledVal(val) {}

    void consume(size_t key, size_t value)
    {
        m_controlledVal.insert(value);
    }

private:
    std::set<size_t>& m_controlledVal;
};

using CircularStringQueue = CircularQueue<std::string>;
using CircularIntQueue = CircularQueue<int>;

class StringQueueProxy : public CircularQueue<std::string>
{
public:
    StringQueueProxy(const CircularQueue<std::string>& ref) : m_queue(ref)
    {
    }

    size_t getSize() const
    {
        return m_queue.m_container.size();
    }
private:
    const CircularQueue<std::string>& m_queue;
};

class IntQueueProxy : public CircularQueue<int>
{
public:
    IntQueueProxy(const CircularQueue<int>& ref) : m_queue(ref)
    {
    }

    size_t getSize()
    {
        return m_queue.m_container.size();
    }
private:
    const CircularQueue<int>& m_queue;
};

class StringQueueBaseProxy : public QueueBase<std::string, std::string,CircularQueue<std::string>, StringConsumer>
{
public:  
    StringQueueBaseProxy() = default;
    StringQueueBaseProxy(const std::string s) : QueueBase(s) {}

    const StringQueueProxy& getQueue() { return m_container; }    
};

class IntQueueBaseProxy : public QueueBase<int, int, CircularQueue<int>, IntConsumer>
{
public:
    IntQueueBaseProxy() = default;
    IntQueueBaseProxy(int i) : QueueBase(i) {}

    const IntQueueProxy& getQueue() { return m_container; }
};

// Function copypasted from cppreference.com
// "busy sleep" while suggesting that other threads run 
// for a small amount of time
void little_sleep(std::chrono::microseconds us)
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;
    do {
        std::this_thread::yield();
    } while (std::chrono::high_resolution_clock::now() < end);
}

TEST(CreateAndInit, CreateQueue_success)
{
    auto stringQueue = std::make_shared<StringQueueBaseProxy>();
    EXPECT_FALSE(stringQueue == nullptr);

    auto intQueue = std::make_shared<IntQueueBaseProxy>();
    EXPECT_FALSE(intQueue == nullptr);

    stringQueue->push("Value 1");
    stringQueue->push("Value 2");
    const auto& stringQueueContainer = stringQueue->getQueue();
    auto sz = stringQueueContainer.getSize();

    EXPECT_EQ(sz, 2);
}

TEST(PushAndPop, SimplePushAndPop_success)
{
    auto queueProcessor = std::make_shared<MultyQueue_NS::MultiQueueProcessor<size_t, size_t, CircularQueue<size_t>, IntConsumer>>();

    std::set<size_t> intSet;
    auto intConsumer = std::make_shared<IntConsumer>(intSet);
    for (int i = 0; i < 100; ++i)
    {
        queueProcessor->Enqueue(0, i);
    }

    queueProcessor->Subscribe(0, intConsumer);
    for (int i = 100; i < 200; ++i)
    {
        queueProcessor->Enqueue(0, i);
    }

    little_sleep(500ms);

    EXPECT_EQ(intSet.size(), 200);

    for (int i = 0; i < 200; ++i)
    {
        EXPECT_TRUE(intSet.count(i) == 1);
    }
}

TEST(PushAndPop, MultyThreadPushAndPop_success)
{
    static constexpr size_t KeyCount = 10;
    static constexpr size_t ProducersPerKey = 10;
    static constexpr size_t ValuesPerKey = 10;

    std::vector<std::set<size_t>> sets(KeyCount);
    std::vector<std::shared_ptr<IntConsumer>> consumers(0);

    std::list<std::thread> threads;

    for (size_t key = 0; key < KeyCount; ++key)
    {
        consumers.push_back(std::make_shared<IntConsumer>(sets[key]));
    }

    auto queueProcessor = std::make_shared<MultyQueue_NS::MultiQueueProcessor<size_t, size_t, CircularQueue<size_t>, IntConsumer>>();
    for (size_t key = 0; key < KeyCount; ++key)
    {
        threads.push_back(std::thread([&queueProcessor, &consumers, key]()
            {
                queueProcessor->Subscribe(key, consumers[key]);
            }));

        for (size_t prod = 0; prod < ProducersPerKey; ++prod)
        {
            threads.push_back(std::thread([&queueProcessor, key, prod]()
                {
                    for (size_t i = prod * ValuesPerKey; i < (prod + 1) * ValuesPerKey; ++i)
                    {
                        queueProcessor->Enqueue(key, i);
                    }
                }));
        }
    }

    for (auto& thread : threads)
    {
        thread.join();
    }

    for (const auto& theSet : sets)
    {
        ASSERT_EQ(theSet.size(), ProducersPerKey * ValuesPerKey);
        for (size_t val = 0; val < ProducersPerKey * ValuesPerKey; ++val)
        {
            ASSERT_TRUE(theSet.count(val) == 1);
        }
    }
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);

    RUN_ALL_TESTS();
}