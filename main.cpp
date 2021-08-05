// main.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "ConsumerBase.h"
#include "Manager.h"

#include <iostream>
#include <string>

class StringConsumer : public MultyQueue_NS::ConsumerBase<std::string, std::string, StringConsumer>
{
public:
    void Consume(const std::string& key, const std::string& value)
    {
        std::cout << "StringConsumer consumed: " << key << " : " << value << std::endl;
    }
};

int main(int argc, char** argv)
{    
    MultyQueue_NS::Manager<std::string, std::string, StringConsumer> queueProcessor;

    StringConsumer stringConsumer;

    queueProcessor.Subscribe(&stringConsumer);

    queueProcessor.Enqueue("Key", "Value");
}