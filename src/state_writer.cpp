#include "config.hpp"
#include "state.hpp"

extern "C" {
#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <sched.h>
#include <stdint.h>
}

#include <thread>
#include <chrono>

using namespace std::chrono;

void* state_writer(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    while (true) {
        auto now = std::chrono::system_clock::now();

        Telemetry::state_mutex.lock();
        for (size_t i = 0; i < Telemetry::NUM_STATE_CHANNELS; i += 1) {
            
            uint64_t now = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
            Telemetry::data_queue.enqueue({
                .timestamp = now,
                .data = *Telemetry::state_telemetry[i],
                .sensor_id = Telemetry::STATE_CHANNEL_START + i,
            });
        }
        std::this_thread::sleep_until(now + std::chrono::milliseconds(Telemetry::STATE_TICK_RATE_MS));
    }
}
