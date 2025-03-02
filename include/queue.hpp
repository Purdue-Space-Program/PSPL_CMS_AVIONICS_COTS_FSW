#pragma once

#include "config.hpp"
#include <mutex>
#include <protocols.hpp>
extern "C" {
#include <semaphore.h>
}

class Queue {
private:
    Telemetry::SensorPacket_t arr[Telemetry::DATA_QUEUE_LENGTH] = {0};

    size_t head;
    size_t tail;

    sem_t sem_empty;
    sem_t sem_full;
    std::mutex mut;

public:
    Queue();
    void enqueue(Telemetry::SensorPacket_t value);
    void dequeue(Telemetry::SensorPacket_t* packet);
};
