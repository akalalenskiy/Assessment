// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ConsumerBase.h"
#include "Manager.h"
#include "Queue.h"

#include <iostream>
#include <string>

using namespace MultyQueue_NS;

class StringConsumer : public ConsumerBase<std::string, std::string, StringConsumer>
{
public:
    void Consume(const std::string& key, const std::string& value)
    {
        std::cout << "StringConsumer consumed: " << key << " : " << value << std::endl;
    }
};

//template<typename TValue>
using CircularStringQueue = CircularQueue<std::string>;

int main(int argc, char** argv)
{   
    auto queueProcessor = std::make_shared<MultyQueue_NS::Manager<std::string, std::string, CircularStringQueue, StringConsumer>>();

    auto stringConsumer = std::make_shared<StringConsumer>();
    auto stringConsumer1 = std::make_shared<StringConsumer>();
    auto stringConsumer2 = std::make_shared<StringConsumer>();
    auto stringConsumer3 = std::make_shared<StringConsumer>();

    queueProcessor->Subscribe(std::string("Key") , stringConsumer);

    queueProcessor->Enqueue("Key", "Value");
    queueProcessor->Enqueue("Key", "Value 1");
    queueProcessor->Enqueue("Key", "Value 2");

    //queueProcessor->Enqueue("Key1", "Value");
    //queueProcessor->Enqueue("Key2", "Value");
    //queueProcessor->Enqueue("Key3", "Value");

    //queueProcessor->Subscribe(std::string("Key1"), stringConsumer);
    //queueProcessor->Subscribe(std::string("Key2"), stringConsumer);
    //queueProcessor->Subscribe(std::string("Key3"), stringConsumer);
}