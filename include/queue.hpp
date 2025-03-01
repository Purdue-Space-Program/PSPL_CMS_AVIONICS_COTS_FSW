#ifndef __QUEUE_HPP__
#define __QUEUE_HPP__

#include "config.hpp"
#include <mutex>
#include <semaphore.h>
#include <protocols.hpp>

class Queue {
private:
    Telemetry::SensorPacket_t arr[Telemetry::DATA_QUEUE_LENGTH] = {0};
    uint32_t size;
    uint32_t front;
    uint32_t back;
    uint32_t count;
    sem_t sema;
    std::mutex q_mut;

public:
    Queue();
    void enqueue(Telemetry::SensorPacket_t value);
    int dequeue(Telemetry::SensorPacket_t* packet);
    bool is_empty();
    uint32_t get_size();
};

#endi
