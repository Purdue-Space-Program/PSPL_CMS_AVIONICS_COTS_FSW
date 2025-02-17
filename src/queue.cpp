#include <queue.hpp>

void Queue::enqueue(SensorPacket_t value) {
    if (count == size) {
        // queue is full
        return;
    }
    arr[back] = value;
    back = (back + 1) % size;  // Circular increment
    count += 1;
}

SensorPacket_t Queue::dequeue() {
    if (count == 0) {
    }
    SensorPacket_t value = arr[front];
    front = (front + 1) % size;  // Circular increment
    count -= 1;
    return value;
}

bool Queue::is_empty() {
    return count == 0;
}

uint32_t Queue::get_size() {
    return count;
}
