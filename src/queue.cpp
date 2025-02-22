#include <queue.hpp>

using namespace Telemetry;

void Queue::enqueue(SensorPacket_t value) {
    if (count == size) {
        // queue is full
        return;
    }

    q_mut.lock();

    arr[back] = value;
    back = (back + 1) % size;
    count += 1;

    q_mut.unlock();
}

SensorPacket_t Queue::dequeue() {
    if (count == 0) {
        // empty
        return;
    }

    q_mut.lock();

    SensorPacket_t value = arr[front];
    front = (front + 1) % size;
    count -= 1;

    q_mut.unlock();

    return value;
}

bool Queue::is_empty() {
    return count == 0;
}

uint32_t Queue::get_size() {
    return count;
}
