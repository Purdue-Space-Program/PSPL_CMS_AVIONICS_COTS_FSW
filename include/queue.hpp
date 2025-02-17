#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include "config.hpp"
#include <protocols.hpp>

using namespace Sensor;

class Queue {
private:
    SensorPacket_t arr[DATA_QUEUE_LENGTH] = {0};
    uint32_t size;
    uint32_t front;
    uint32_t back;
    uint32_t count;

public:
    Queue() : size(DATA_QUEUE_LENGTH), front(0), back(0), count(0) {}
    void enqueue(SensorPacket_t value);
    SensorPacket_t dequeue();
    bool is_empty();
    uint32_t get_size();
};

#endif
