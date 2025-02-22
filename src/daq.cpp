#include <pthread.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <sched.h>
#include <time.h>
#include <daqhats/daqhats.h>
#include <daqhats/mcc118.h>

#include "config.hpp"
#include "queue.hpp"
#include "state.hpp"

uint64_t Telemetry::he_pressure = 0;
uint64_t Telemetry::fu_pressure = 0;
uint64_t Telemetry::ox_pressure = 0;
uint64_t Telemetry::tc_0 = 0;
uint64_t Telemetry::tc_1 = 0;

Queue Telemetry::data_queue = Queue();

void* daq(void* arg) {
    struct sched_param param;
    param.sched_priority = 99; // highest prio
    pthread_setschedparam(pthread_self(), SCHED_RR, &param);
    // TODO: FDIR

    int result = mcc118_open(Daq::ADDRESS);
    if (result != RESULT_SUCCESS) {
        // TODO: FDIR
    }

    uint32_t options = OPTS_DEFAULT;
    double value;
    while (true) {
        struct timespec time;
        clock_gettime(CLOCK_MONOTONIC, &time);
        // TODO: FDIR
        time.tv_nsec += Telemetry::POLL_RATE_MS * 1000000;
        if (time.tv_nsec >= 1000000000) {
            time.tv_sec += 1;
            time.tv_nsec -= 1000000000;
        }

        for (uint8_t ch = Telemetry::AI_CHANNEL_START; ch < Telemetry::NUM_AI_CHANNELS; ch += 1) {
            struct timespec timestamp;

            result = mcc118_a_in_read(Daq::ADDRESS, ch, options, &value);
            clock_gettime(CLOCK_MONOTONIC, &timestamp);

            Telemetry::data_queue.enqueue({
                .sensor_id = ch,
                .timestamp = timestamp.tv_sec * 1000000UL + timestamp.tv_nsec / 1000UL,
                .data = *(uint64_t*)(&value),
            });
        }

        clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &time, NULL);
        // TODO: FDIR
    }
}
