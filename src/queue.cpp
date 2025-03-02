#include "config.hpp"
#include "protocols.hpp"
#include <queue.hpp>

using namespace Telemetry;

Queue::Queue() {
    sem_init(&sem_empty, 0, Telemetry::DATA_QUEUE_LENGTH);
    sem_init(&sem_full, 0, 0);
    head = 0;
    tail = 0;
}

void Queue::enqueue(SensorPacket_t value) {
    sem_wait(&sem_empty);

    mut.lock();
    arr[tail++] = value;
    tail %= Telemetry::DATA_QUEUE_LENGTH;
    mut.unlock();

    sem_post(&sem_full);
}

void Queue::dequeue(SensorPacket_t* packet) {
    sem_wait(&sem_full);

    mut.lock();
    *packet = arr[head++];
    head %= Telemetry::DATA_QUEUE_LENGTH;
    mut.unlock();

    sem_post(&sem_empty);
}
