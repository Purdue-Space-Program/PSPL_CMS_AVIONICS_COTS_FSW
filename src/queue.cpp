
#include "config.hpp"
#include "protocols.hpp"
#include <queue.hpp>
#include <semaphore.h>

using namespace Telemetry;

Queue::Queue() : size(Telemetry::DATA_QUEUE_LENGTH), front(0), back(0), count(0) {
    sem_init(&sema, 0, 0);
}

void Queue::enqueue(SensorPacket_t value) {
    if (count == size) {
        // queue is full
        return;
    }

    q_mut.lock();

    arr[back] = value;
    back = (back + 1) % size;
    count += 1;

    if (count == DATA_QUEUE_LENGTH) {
        sem_post(&sema);
    }

    q_mut.unlock();
}

int Queue::dequeue(SensorPacket_t* packet) {
    if (count == 0) {
        // empty
        return -1;
    } else if (count == DATA_QUEUE_LENGTH) {
        sem_wait(&sema);
    }

    q_mut.lock();

    SensorPacket_t value = arr[front];
    front = (front + 1) % size;
    count -= 1;

    q_mut.unlock();

    *packet = value;

    return 0;
}

bool Queue::is_empty() {
    return count == 0;
}

uint32_t Queue::get_size() {
    return count;
}
