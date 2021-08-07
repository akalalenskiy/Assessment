#pragma once

namespace MultyQueue_NS
{

enum class FullQueuePolicy
{
    WaitForSpace,
    IncreaseCapacity,
    ThrowException
};

}
