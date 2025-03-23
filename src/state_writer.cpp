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

volatile uint64_t* state_telemetry[] = {
    (uint64_t*)&BB_State::bb_fu_state,
    (uint64_t*)&BB_State::bb_ox_state,
    &BB_State::bb_fu_pos,
    &BB_State::bb_ox_pos,
    &BB_State::bb_fu_upper_setp,
    &BB_State::bb_fu_lower_setp,
    &BB_State::bb_ox_upper_setp,
    &BB_State::bb_ox_lower_setp,
};

void* state_writer(void* arg) {
    struct sched_param param;
    param.sched_priority = 11; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    while (true) {
        auto now = system_clock::now();

        for (size_t i = 0; i < Telemetry::NUM_STATE_CHANNELS; i += 1) {
            Telemetry::state_mutex.lock();
            uint64_t n = duration_cast<microseconds>(system_clock::now().time_since_epoch()).count();
            Telemetry::data_queue.enqueue({
                .timestamp = n,
                .data = *(state_telemetry[i]),
                .sensor_id = Telemetry::STATE_CHANNEL_START + i,
            });
            Telemetry::state_mutex.unlock();
        }

        std::this_thread::sleep_until(now + milliseconds(Telemetry::STATE_TICK_RATE_MS));
    }
}
