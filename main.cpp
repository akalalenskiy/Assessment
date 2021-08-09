// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <chrono>
#include <iostream>
#include <list>
#include <set>
#include <string>
#include <thread>

#include "CircularQueue.h"
#include "ConsumerBase.h"
#include "MultiQueueProcessor.h"
#include "Queue.h"

using namespace std::chrono_literals;
using namespace MultyQueue_NS;

std::list<std::string> strings1;
std::list<std::string> strings2;
std::list<std::string> strings3;

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

class StringConsumer : public ConsumerBase<std::string, std::string, StringConsumer>
{
public:
    StringConsumer(std::list<std::string>& val) : m_strings(val) {}

    void consume(const std::string& key, std::string value)
    {
        m_strings.emplace_back(std::move(value));
    }

private:
    std::list<std::string>& m_strings;
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

int main(int argc, char** argv)
{   
    auto queueProcessor = std::make_shared<MultyQueue_NS::MultiQueueProcessor<std::string, std::string, CircularStringQueue, StringConsumer>>();

    auto stringConsumer1 = std::make_shared<StringConsumer>(strings1);
    auto stringConsumer2 = std::make_shared<StringConsumer>(strings2);
    auto stringConsumer3 = std::make_shared<StringConsumer>(strings3);


    queueProcessor->enqueue("Key1", "Value 1-0");
    queueProcessor->enqueue("Key1", "Value 1-1");
    queueProcessor->enqueue("Key1", "Value 1-2");

    queueProcessor->enqueue("Key2", "Value 2-0");
    queueProcessor->enqueue("Key2", "Value 2-1");
    queueProcessor->enqueue("Key2", "Value 2-2");

    queueProcessor->subscribe(std::string("Key1"), stringConsumer1);
    queueProcessor->subscribe(std::string("Key2"), stringConsumer2);
    queueProcessor->subscribe(std::string("Key3"), stringConsumer3);

    queueProcessor->enqueue("Key3", "Value 3-0");
    queueProcessor->enqueue("Key3", "Value 3-1");
    queueProcessor->enqueue("Key3", "Value 3-2");

    const auto printList = [](std::list<std::string>& strings) {
        for (const auto& ss : strings)
        {
            std::cout << ss << std::endl;
        }
    };

    little_sleep(250ms);

    std::cout << "== Content of Key 1 ==================" << std::endl;
    printList(strings1);

    std::cout << "== Content of Key 2 ==================" << std::endl;
    printList(strings2);

    std::cout << "== Content of Key 3 ==================" << std::endl;
    printList(strings3);

    static constexpr size_t KeyCount = 10;
    static constexpr size_t ProducersPerKey = 10;
    static constexpr size_t ValuesPerKey = 10;

    std::cout << "Performing multi-threading test." <<
        ProducersPerKey << " producers per each of " << KeyCount << " will be pushing " <<
        ValuesPerKey*KeyCount << " different values" << std::endl;

    {
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
                    queueProcessor->subscribe(key, consumers[key]);
                }));

            for (size_t prod = 0; prod < ProducersPerKey; ++prod)
            {
                threads.push_back(std::thread([&queueProcessor, key, prod]()
                    {
                        for (size_t i = prod * ValuesPerKey; i < (prod + 1) * ValuesPerKey; ++i)
                        {
                            queueProcessor->enqueue(key, i);
                        }
                    }));
            }
        }

        for (auto& thread : threads)
        {
            thread.join();
        }
    }

    std::cout << "Press a key" << std::endl;
    getchar();
}